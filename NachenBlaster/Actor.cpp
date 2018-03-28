#include "Actor.h"

//////////////////////////////////////////////////////////////////////////////////
// HELPER FUNCTIONS 
//////////////////////////////////////////////////////////////////////////////////

double starSize()
{
	double temp = randInt(5, 50);
	temp /= 100.0;
	return temp;
}
double euclidianDistance(double x1, double y1, double x2, double y2)
{
	double tempX = x2 - x1;
	double tempY = y2 - y1;
	return sqrt((tempX * tempX) + (tempY * tempY));
}
bool isAlien(const Actor* target)
{
	if (target->getActorID() == IID_SMALLGON ||
		target->getActorID() == IID_SMOREGON ||
		target->getActorID() == IID_SNAGGLEGON)
		return true;
	return false;
}
bool isFriendlyProjectile(const Actor* target)
{
	if (target->getActorID() == IID_CABBAGE)	// cabbages are friendly 
		return true;
	if (target->getActorID() == IID_TORPEDO &&	// torpedoes moving right are friendly
		target->getDirection() == 0)
		return true;
	return false;
}
bool isEnemyProjectile(const Actor* target)
{
	if (target->getActorID() == IID_TURNIP)	// turnips are from enemies
		return true;
	if (target->getActorID() == IID_TORPEDO &&	// torpedoes moving left are from enemies
		target->getDirection() == 180)
		return true;
	return false;
}
bool isGoodie(const Actor* target)
{
	if (target->getActorID() == IID_REPAIR_GOODIE ||
		target->getActorID() == IID_LIFE_GOODIE   ||
		target->getActorID() == IID_TORPEDO_GOODIE )
		return true;
	return false;
}

//////////////////////////////////////////////////////////////////////////////////
// ACTOR IMPLEMENTATION
//////////////////////////////////////////////////////////////////////////////////

Actor::Actor(const int& imageID, const double& startX, const double& startY, 
			 StudentWorld* world, Direction dir, double size, unsigned int depth)
:GraphObject(imageID, startX, startY, dir, size, depth)
{
	m_alive = true;
	m_actorID = imageID;
	m_scorePoints = 0;
	m_world = world;
	m_health = 5;
}
void Actor::collide(Actor& other)	// if there is a collision, look at the proper collisionProperties
{
	if (euclidianDistance(getX(), getY(), other.getX(), other.getY()) < .75 * (getRadius() + other.getRadius()))
	{	
		if (other.isAlive() && isAlive())	// make sure both things are alive before colliding
			collisionProperties(other);
	}
}

	// helper function definitions
void Actor::setHealth(int amt)
{
	m_health = amt;
}
int  Actor::getHealth() const
{
	return m_health;
}
void Actor::takeDamage(int amt)
{
	m_health -= amt;
}
int  Actor::getActorID() const
{
	return m_actorID;
}   // get an actor's ID
void Actor::kill()
{
	m_alive = false;
}			// set an actor's state to dead
bool Actor::isAlive() const
{
	return m_alive;
}		// check if an actor is alive (true) or dead (false)
void Actor::setScore(int amt)				// set an actors points scoring mechanism to true
{
	m_scorePoints = amt;
}   
int  Actor::getScore() const
{
	return m_scorePoints;
}
inline StudentWorld* Actor::getWorld() const
{
	return m_world;
}

//////////////////////////////////////////////////////////////////////////////////
// STAR IMPLEMENTATION
//////////////////////////////////////////////////////////////////////////////////

Star::Star(double startY, StudentWorld* world)
	:Actor(IID_STAR, VIEW_WIDTH-1, startY, world, 0, starSize(), 3)
{
}

Star::Star(double startX, double startY, StudentWorld* world)
	:Actor(IID_STAR, startX, startY, world, 0, starSize(), 3)
{
}

void Star::doSomething()	// stars move left 1 pixel/tick and die once they fall off screen
{
	if (getX() <= 0)
	{
		kill();
	}
	else
		moveTo(getX() - 1, getY());
}

//////////////////////////////////////////////////////////////////////////////////
// EXPLOSION IMPLEMENTATION
//////////////////////////////////////////////////////////////////////////////////

Explosion::Explosion(double startX, double startY, StudentWorld* world)
	:Actor(IID_EXPLOSION, startX, startY, world, 0, 1.0, 0)
{
	const int EXPLOSION_LENGTH = 3;
	m_lifeTime = EXPLOSION_LENGTH;
}

void Explosion::doSomething()
{
	if (m_lifeTime <= 0)	
		kill();
	else
	{
		setSize(getSize() * 1.5);
		m_lifeTime--;
	}
}

//////////////////////////////////////////////////////////////////////////////////
// NACHENBLASTER IMPLEMENTATION
//////////////////////////////////////////////////////////////////////////////////

NachenBlaster::NachenBlaster(StudentWorld* world)
	:Actor(IID_NACHENBLASTER, 0, 128, world, 0, 1.0, 0), m_cabbageEnergy(30), m_nOfTorpedoes(0)
{
	setHealth(50);
}

void NachenBlaster::doSomething()
{
	if (!isAlive())	// if dead, don't do anything
		return;
	if (m_cabbageEnergy < 30)	// increase energy each tick
		m_cabbageEnergy++;
	int ch;
	if (getWorld()->getKey(ch))	// if user pressed something, shoot and/or move
	{
		shootObject(ch);
		moveShip(ch);
	}
}

void NachenBlaster::collisionProperties(Actor& other)	// when nachenblaster collides with something, 
{
	// if we hit an object that does something, call its collide function.
	if (isAlien(&other) || isEnemyProjectile(&other) || isGoodie(&other))
			other.collide(*this);
	// if we hit a star, explosion, or friendly projectile (AKA anything else), do nothing
}

	// helper functions, inlined because we only make one user, so not memory intensive
inline void NachenBlaster::shootObject(const int& ch)
{
	switch (ch)
	{
	case KEY_PRESS_SPACE:		// shoot a cabbage
	{
		if (m_cabbageEnergy < 5) break;	// if not enough energy, don't do anything
		m_cabbageEnergy -= 5;
		Cabbage *temp = new Cabbage(getX() + 12, getY(), getWorld());
		getWorld()->createActor(temp);
		getWorld()->playSound(SOUND_PLAYER_SHOOT);
		break;
	}
	case KEY_PRESS_TAB:		// shoot a torpedo
	{
		if (m_nOfTorpedoes <= 0)	break;// check that we have a torpedo
		m_nOfTorpedoes--;
		FTorpedoProjectile *temp = new FTorpedoProjectile(getX() + 12, getY(), getWorld(), 0);
		getWorld()->createActor(temp);
		getWorld()->playSound(SOUND_TORPEDO);
		break;
	}
	}
}
inline void NachenBlaster::moveShip(const int& dir)
{
	// if U/D/L/R and if we can move, then move
	switch (dir)
	{
	case KEY_PRESS_DOWN:
	{
		if (getY() >= 6)
			moveTo(getX(), getY() - 6);
		break;
	}
	case KEY_PRESS_UP:
	{
		if (getY() < VIEW_HEIGHT - 6)
			moveTo(getX(), getY() + 6);
		break;
	}
	case KEY_PRESS_LEFT:
	{
		if (getX() >= 6)
			moveTo(getX() - 6, getY());
		break;
	}
	case KEY_PRESS_RIGHT:
	{
		if (getX() < VIEW_WIDTH - 6)
			moveTo(getX() + 6, getY());
		break;
	}
	}
}
void NachenBlaster::takeDamage(int amt)
{
	Actor::takeDamage(amt); 
	if (getHealth() <= 0)
		kill();
	if (getHealth() > 50)	// health can't go above 50
		setHealth(50);
}
void NachenBlaster::incTorpedoes(int amt)
{
	m_nOfTorpedoes += amt;
}
int NachenBlaster::getCabbageEnergy() const
{
	return m_cabbageEnergy;
}
int NachenBlaster::getNOfTorpedoes() const
{
	return m_nOfTorpedoes;
}

//////////////////////////////////////////////////////////////////////////////////
// PROJECTILE IMPLEMENTATION
//////////////////////////////////////////////////////////////////////////////////

Projectile::Projectile(int imageID, double startX, double startY, StudentWorld* world, int dir)
	:Actor(imageID, startX, startY, world, dir, .5, 1)
{
}

void Projectile::doSomething()
{
	if (!isAlive()) return;
	if (getX() < 0 || getX() > VIEW_WIDTH - 1)	// if we've flown out of bounds, kill the object
		kill();
	else
	{
		int direction, nOfPixels;
		bool doesRotate;
		doSomethingProperties(direction, nOfPixels, doesRotate);	// assigns values to arguments
		if (doesRotate)	// rotate if cabbage/turnip
			setDirection(getDirection() + 20);
		if (direction == LEFT)	// move projectile left
			moveTo(getX() - nOfPixels, getY());
		else // move projectile right
			moveTo(getX() + nOfPixels, getY());
	}
}


/////////////////////////////////////////
// CABBAGE PROJECTILE IMPLEMENTATION
/////////////////////////////////////////

Cabbage::Cabbage(double startX, double startY, StudentWorld* world)
	:Projectile(IID_CABBAGE, startX, startY, world, 0)
{}

void Cabbage::collisionProperties(Actor& other)	
{
	// if collides with an alien, inflict damage and set state to dead
	if (isAlien(&other)) 
	{
		other.takeDamage(2);
		kill();
	}
}

void Cabbage::doSomethingProperties(int &dir, int& nOfPixels, bool& doesRotate)
{
	dir = RIGHT;
	nOfPixels = 8;
	doesRotate = true;
}

/////////////////////////////////////////
// TURNIP PROJECTILE IMPLEMENTATION
/////////////////////////////////////////

Turnip::Turnip(double startX, double startY, StudentWorld* world)
	:Projectile(IID_TURNIP, startX, startY, world, 0)
{}

void Turnip::collisionProperties(Actor& other)
{
	// if proper collision, inflict damage and set state to dead
	other.takeDamage(2);
	kill();
}

void Turnip::doSomethingProperties(int &dir, int& nOfPixels, bool& doesRotate)
{
	dir = LEFT;
	nOfPixels = 6;
	doesRotate = true;
}

/////////////////////////////////////////
// FLATULENCE TORPEDO PROJECTILE IMPLEMENTATION
/////////////////////////////////////////

FTorpedoProjectile::FTorpedoProjectile(double startX, double startY, StudentWorld* world, int Dir)
	:Projectile(IID_TORPEDO, startX, startY, world, Dir)
{}	

void FTorpedoProjectile::collisionProperties(Actor& other)
{
	if (getDirection() == 0 && isAlien(&other))	
	{	// if fired from nachenblaster and attacking an alien
		other.takeDamage(8);
		kill();
	}
	else if (getDirection() == 180 && other.getActorID() == IID_NACHENBLASTER)
	{	// if fired from alien and attacking with NB
		other.takeDamage(8);
		kill();
	}
}

void FTorpedoProjectile::doSomethingProperties(int& dir, int& nOfPixels, bool& doesRotate)
{
	doesRotate = false;
	nOfPixels = 8;
	if (isFriendlyProjectile(this))
		dir = RIGHT;
	else	// if enemy torpedo
		dir = LEFT;
}

//////////////////////////////////////////////////////////////////////////////////
// GOODIES IMPLEMENTATION
//////////////////////////////////////////////////////////////////////////////////

Goodie::Goodie(int imageID, double startX, double startY, StudentWorld* world)
	:Actor(imageID, startX, startY, world, 0, .5, 1)
{}

void Goodie::doSomething()
{
	moveTo(getX() - .75, getY() - .75);
}

/////////////////////////////////////////
// EXTRA LIFE GOODIE IMPLEMENTATION
/////////////////////////////////////////

ExtraLife::ExtraLife(double startX, double startY, StudentWorld* world)
	:Goodie(IID_LIFE_GOODIE, startX, startY, world)
{}

void ExtraLife::collisionProperties(Actor& other)
{
	kill();
	setScore(100);
	getWorld()->playSound(SOUND_GOODIE);
	getWorld()->incLives();
}

/////////////////////////////////////////
// REPAIR HEALTH GOODIE IMPLEMENTATION
/////////////////////////////////////////

Repair::Repair(double startX, double startY, StudentWorld* world)
	:Goodie(IID_REPAIR_GOODIE, startX, startY, world)
{}

void Repair::collisionProperties(Actor& other)
{
	kill();
	setScore(100);
	getWorld()->playSound(SOUND_GOODIE);
	other.takeDamage(-10);
}

/////////////////////////////////////////
// FLATULENCE TORPEDO GOODIE IMPLEMENTATION
/////////////////////////////////////////

FTorpedoGoodie::FTorpedoGoodie(double startX, double startY, StudentWorld* world)
	:Goodie(IID_TORPEDO_GOODIE, startX, startY, world)
{}

void FTorpedoGoodie::collisionProperties(Actor& other)
{
	kill();
	setScore(100);
	getWorld()->playSound(SOUND_GOODIE);
	getWorld()->getUser()->incTorpedoes(5);	// call NBs inc torpedoes function
}

//////////////////////////////////////////////////////////////////////////////////
// ALIEN IMPLEMENTATION
//////////////////////////////////////////////////////////////////////////////////

Alien::Alien(int imageID, double startY, StudentWorld* world)
	:Actor(imageID, VIEW_WIDTH - 1, startY, world, 0, 1.5, 1)
{
	m_flightPlanLen = 0;
	m_travelSpeed = 2.0;
	chooseRandDirection();
	int temp = 5 * (1 + (getWorld()->getLevel() - 1)*.1);
	setHealth(temp);
}

void Alien::collisionProperties(Actor& other)		// for when the NB runs into it
{
	takeDamage(100);
	setScore(250);
	other.takeDamage(5);
	if (getHealth() <= 0)
	{
		// check if dead here or else alien may collide twice before student world will clean it up
		setScore(250);
		kill();
		possiblyDropItem();
	}
}

// decide if dead, move, shoot
void Alien::doSomething()
{
	// check if still alive and whether or not killed by player
	if (getHealth() <= 0)
	{
		setScore(250);	// score of 250 for small/smoregons
		kill();
		possiblyDropItem();	// snagglegon's dropItem function sets score to 1000 instead of 250
	}
	if (getX() <= 0) kill();

	// change/set flight plan if we have to
	if (m_flightPlanLen <= 0)	chooseRandDirection();
	if (getY() <= 0) chooseOtherDirection(DOWN_LEFT);
	else if (getY() >= VIEW_HEIGHT - 1) chooseOtherDirection(UP_LEFT);

	// check if you need to ram/shoot
	if (getWorld()->getUser()->getX() < getX())		// if user is left of alien AND 
		if (getWorld()->getUser()->getY() >= (getY() - 4) &&	// if user is within 4 pixels of the
			getWorld()->getUser()->getY() <= (getY() + 4))		// alien's height
		{
			if (randInt(1, (20 / getWorld()->getLevel()) + 5) == 1)	// then 1/20 chance of shooting
			{
				shoot();
				return;
			}
			else if (randInt(1, (20 / getWorld()->getLevel()) + 5) == 1)
				ram();
		}
	// move the alien
	moveAlien();
}

	// helper functions
void Alien::takeDamage(int amt)
{
	Actor::takeDamage(amt);
	if (getHealth() > 0)		// don't check if dead here: check it in doSomething
		getWorld()->playSound(SOUND_BLAST);
}
void Alien::shoot()
{
	Turnip* temp = new Turnip(getX() - 14, getY(), getWorld());
	getWorld()->createActor(temp);
	getWorld()->playSound(SOUND_ALIEN_SHOOT);
}

	// private functions
void Alien::moveAlien()	
{
	// decrease flight plan length each time we move
	m_flightPlanLen--;			

	// move alien left, up left, or down left
	if (m_dir == LEFT)
		moveTo(getX() - m_travelSpeed, getY());	
	else if (m_dir == DOWN_LEFT)
		moveTo(getX() - m_travelSpeed, getY() - m_travelSpeed);	
	else if (m_dir == UP_LEFT)
		moveTo(getX() - m_travelSpeed, getY() + m_travelSpeed);
}

int Alien::getDirection() const
{
	return m_dir;
}

void Alien::setDirection(int dir)
{
	m_dir = dir;
}

double Alien::getTravelSpeed() const
{
	return m_travelSpeed;
}

void Alien::setTravelSpeed(double amt)
{
	m_travelSpeed = amt;
}

void Alien::setFlightPlanLen(int amt)
{
	m_flightPlanLen = amt;
}

void Alien::chooseRandDirection()
{
	m_dir = randInt(DOWN_LEFT, UP_LEFT);	// directions are random ints from DOWN_LEFT to UP_LEFT
	m_flightPlanLen = randInt(1, 32);
}

void Alien::chooseOtherDirection(int notThisDir)
{
	if (notThisDir == DOWN_LEFT)
		m_dir = randInt(LEFT, UP_LEFT);
	else if (notThisDir == UP_LEFT)
		m_dir = randInt(DOWN_LEFT, LEFT);
	m_flightPlanLen = randInt(1, 32);
}

/////////////////////////////////////////
// SMALLGON IMPLEMENTATION
/////////////////////////////////////////

Smallgon::Smallgon(double startY, StudentWorld* world)
	:Alien(IID_SMALLGON, startY, world)
{}

/////////////////////////////////////////
// SMOREGON IMPLEMENTATION
/////////////////////////////////////////

Smoregon::Smoregon(double startY, StudentWorld* world)
	:Alien(IID_SMOREGON, startY, world)
{}

void Smoregon::possiblyDropItem()
{
	if (randInt(1, 3) == 1)	// 1/3 chance to drop
	{
		if (randInt(1, 2) == 1)	// 1/2 chance to drop repair goodie
		{
			Repair* temp = new Repair(getX(), getY(), getWorld());
			getWorld()->createActor(temp);
		}
		else	// 1/2 chance to drop fTorpedo goodie
		{
			FTorpedoGoodie* temp = new FTorpedoGoodie(getX(), getY(), getWorld());
			getWorld()->createActor(temp);
		}
	}
}

void Smoregon::ram()
{
	setDirection(LEFT);
	setTravelSpeed(5.0);
	setFlightPlanLen(VIEW_WIDTH);
}

/////////////////////////////////////////
// SNAGGLEGON IMPLEMENTATION
/////////////////////////////////////////

Snagglegon::Snagglegon(double startY, StudentWorld* world)
	:Alien(IID_SNAGGLEGON, startY, world)
{
	int temp = 10 * (1 + (getWorld()->getLevel() - 1)*.1);
	setHealth(temp);
}

void Snagglegon::collisionProperties(Actor& other)
{
	takeDamage(100);
	other.takeDamage(15);
	if (getHealth() <= 0)
	{
		// check if dead here or else alien may collide twice before student world will clean it up
		setScore(1000);
		kill();
		possiblyDropItem();
	}
}

void Snagglegon::moveAlien()
{
	// when first created, start moving down
	if (getX() == VIEW_WIDTH - 1)
		setDirection(DOWN_LEFT);


	// if we run into top/bottom of screen, change direction
	if (getY() <= 0) chooseOtherDirection(DOWN_LEFT);
	else if (getY() >= VIEW_HEIGHT - 1) chooseOtherDirection(UP_LEFT);

	// move alien left, up left, or down left
	int speed = getTravelSpeed();
	if (getDirection() == DOWN_LEFT)
		moveTo(getX() - speed, getY() - speed);
	else if (getDirection() == UP_LEFT)
		moveTo(getX() - speed, getY() + speed);
}

void Snagglegon::shoot()
{
	FTorpedoProjectile* temp = new FTorpedoProjectile(getX() - 14, getY(), getWorld(), 180);
	getWorld()->createActor(temp);
	getWorld()->playSound(SOUND_TORPEDO);
}

void Snagglegon::possiblyDropItem()
{
	setScore(1000);		// snagglegons score 1000 points on death instead of 250
	if (randInt(1, 6) == 1)	// 1/6 chance to drop
	{
		ExtraLife* temp = new ExtraLife(getX(), getY(), getWorld());
		getWorld()->createActor(temp);
	}
}

