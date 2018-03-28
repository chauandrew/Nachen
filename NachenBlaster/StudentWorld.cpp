#include "StudentWorld.h"
#include "GameConstants.h"
#include <string>
#include <vector>
#include <sstream>
using namespace std;

GameWorld* createStudentWorld(string assetDir)
{
	return new StudentWorld(assetDir);
}

StudentWorld::StudentWorld(string assetDir)
: GameWorld(assetDir)
{
	// initialize member variables to harmless things
	m_user = nullptr;
	m_nAliensOnScreen = 0;
	m_maxNOfAliens = 0;
	m_nOfAliensLeft = 0;
}

StudentWorld::~StudentWorld()
{
	cleanUp();
}

// fill the world with stars, set level parameters, display status line, create a user
int StudentWorld::init()
{
	m_user = new NachenBlaster(this);
	createInitialStars();
	m_nOfAliensLeft = (6 + (4 * getLevel()));
	m_maxNOfAliens  = (4 + (.5 * getLevel()));
	m_nAliensOnScreen = 0;
	displayStatusLine();
    return GWSTATUS_CONTINUE_GAME;
}

// runs every game tick
int StudentWorld::move()
{
	displayStatusLine();	// update status bar each tick
	possiblyCreateStar();	// chance to create a new star
	possiblyCreateAlien();	// create a random new alien if it needs to be created
	checkFriendlyProjectiles();	// check if friendly projectiles hit anything 
	m_user->doSomething();	// take user input

	// make every actor do something: check if user collides with enemies, projectiles, or goodies
	for (unsigned int i = 0; i < m_allActors.size(); ++i)
	{
		m_user->collide(*(m_allActors[i]));		// see if user hits anything before and after moving
		m_allActors[i]->doSomething();
		m_user->collide(*(m_allActors[i]));
	}
	// check if projectiles hit AFTER doing their action
	checkFriendlyProjectiles();

	removeDeadActors();		// remove any actors that need to be removed
	// return game status
	if (m_nOfAliensLeft <= 0) { playSound(SOUND_FINISHED_LEVEL);  return GWSTATUS_FINISHED_LEVEL; }
	else if (m_user->isAlive())	return GWSTATUS_CONTINUE_GAME;
	else /*>~~~(>__<)~~~~<*/  { decLives();	return GWSTATUS_PLAYER_DIED; }
}

// delete user and everything in the actor vector
void StudentWorld::cleanUp()
{
	delete m_user;
	m_user = nullptr;	// so that if we try to delete later, nothing bad happens
	for (unsigned int i = 0; i < m_allActors.size(); ++i)
		delete m_allActors[i];
	while (!m_allActors.empty())	// while vector isn't empty
	{
		vector<Actor*>::iterator it = m_allActors.begin();
		m_allActors.erase(it);
	}
}

///////////////////////////////////
// Helper Functions
///////////////////////////////////

NachenBlaster* StudentWorld::getUser() const
{
	return m_user;
}
void StudentWorld::createInitialStars()
{
	for (int i = 0; i < 30; ++i)	// create 30 stars with random positions
	{
		Star* temp = new Star(randInt(0, VIEW_WIDTH - 1), randInt(0, VIEW_HEIGHT - 1), this);
		m_allActors.push_back(temp);
	}
}
void StudentWorld::displayStatusLine()
{
	int cEnergy = m_user->getCabbageEnergy() * 10 / 3;
	ostringstream temp, cabbages;
	temp.setf(ios::fixed);
	temp.precision(2);
	if (cEnergy < 10)
		cabbages << '0' << cEnergy;
	else
		cabbages << cEnergy;
	temp
		<< "Lives: "	 << getLives()				<<  "   "
		<< "Health: "	 << m_user->getHealth() *2	<< "%   "
		<< "Score: "	 << getScore()				<<  "   "
		<< "Level: "	 << getLevel()				<<  "   "
		<< "Cabbages: "	 << cabbages.str()			<< "%   "
		<< "Torpedoes: " << m_user->getNOfTorpedoes();
	setGameStatText(temp.str());
}
void StudentWorld::decrAliensLeft()
{
	m_nOfAliensLeft--;
}
void StudentWorld::createActor(Actor* newActor)
{
	m_allActors.push_back(newActor);
}
void StudentWorld::removeDeadActors()
{
	for (unsigned int i = 0; i < m_allActors.size(); ++i)
	{
		if (!m_allActors[i]->isAlive()) // if the actor is dead
		{
			if (isAlien(m_allActors[i]))	// if actor was an alien
			{
				m_nAliensOnScreen--;
				// if dead alien is worth points, increase score and kill counter and replace with explosion
				if (m_allActors[i]->getScore() != 0)	
				{
					m_nOfAliensLeft--;
					increaseScore(m_allActors[i]->getScore());
					playSound(SOUND_DEATH);
					// replace existing actor with explosion
					Explosion* temp = new Explosion(m_allActors[i]->getX(), m_allActors[i]->getY(), this);
					delete m_allActors[i];
					m_allActors[i] = temp;
					continue;	// continue so we don't delete the explosion
				}
			}
			// delete the actor and then take out its place from the vector
			vector<Actor*>::iterator it = m_allActors.begin();
			it += i;
			delete m_allActors[i];
			m_allActors.erase(it);
			--i;	// decrement i so that we don't skip an element in the array
		}
	}
}
void StudentWorld::possiblyCreateStar()
{
	int chance = randInt(1, 15);
	if (chance != 1)		// 14/15 chance return
		return;
	Actor* tempStar = new Star(randInt(0, VIEW_HEIGHT - 1), this);
	m_allActors.push_back(tempStar);
}
void StudentWorld::possiblyCreateAlien()
{
	if (m_nAliensOnScreen >= min(m_maxNOfAliens, m_nOfAliensLeft))	// if max aliens on screen, do nothing
		return;
	// if we CAN introduce an alien, create a new one
	int smallChance   = 60;
	int smoreChance   = 20 + (getLevel() * 5);
	int snaggleChance = 5 + (getLevel() * 10);
	int totalChance = smallChance + smoreChance + snaggleChance;
	int test = randInt(0, totalChance);
	if (test < smallChance)	// chance for a smallgon
	{
		Actor* tempAlien = new Smallgon(randInt(0, VIEW_HEIGHT - 1), this);
		m_allActors.push_back(tempAlien);
	}
	else if (test < smallChance + smoreChance)	// chance for a smoregon
	{
		Actor* tempAlien = new Smoregon(randInt(0, VIEW_HEIGHT - 1), this);
		m_allActors.push_back(tempAlien);
	}
	else	// chance for a snagglegon
	{
		Actor* tempAlien = new Snagglegon(randInt(0, VIEW_HEIGHT - 1), this);
		m_allActors.push_back(tempAlien);
	}
	m_nAliensOnScreen++;
}
void StudentWorld::checkFriendlyProjectiles()
{
	for (unsigned int i = 0; i < m_allActors.size(); ++i)
		if (isFriendlyProjectile(m_allActors[i]))
			for (unsigned int j = 0; j < m_allActors.size(); ++j)
			{
				if (!m_allActors[i]->isAlive())
					break;
				m_allActors[i]->collide(*(m_allActors[j]));
			}
}
