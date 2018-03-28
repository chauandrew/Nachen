#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "Actor.h"
#include <string>
#include <vector>
#include <sstream>

class Actor;
class NachenBlaster;
class StudentWorld : public GameWorld
{
public:
    StudentWorld(std::string assetDir);
	~StudentWorld();
    virtual int init();
    virtual int move();
    virtual void cleanUp();

	// helper functions
	NachenBlaster* getUser() const;	// returns the user
	void createInitialStars();	// creates stars for initialization
	void displayStatusLine();	// creates and displays the status line
	void decrAliensLeft();	// decrease nOfAliens left to kill
	void createActor(Actor* newActor);	// adds a new actor to actor vector
	void removeDeadActors();	// removes any dead actors from the vector
	void possiblyCreateStar();	// chance of adding a new star
	void possiblyCreateAlien();	// adds a randomly selected alien if there's space for it
	void checkFriendlyProjectiles();  // checks if friendly projectiles hit any aliens

private:
	int m_nAliensOnScreen;	// number of aliens on screen
	int m_maxNOfAliens;		// max aliens on screen for given level
	int m_nOfAliensLeft;	// number of aliens left until level is over
	std::vector<Actor*> m_allActors;
	NachenBlaster* m_user;
};

#endif // STUDENTWORLD_H_