#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"
#include "StudentWorld.h"
#include <cmath>



// Direction IDs for projectiles/alien movement (i'm not allowed to modify GameConstants.h)
const int RIGHT     = 0; 
const int DOWN_LEFT = 1;
const int LEFT      = 2;	
const int UP_LEFT   = 3;

///////////////////////////////////////////////////////////////
// HELPER FUNCTION PROTOTYPES
///////////////////////////////////////////////////////////////

class Actor;
double euclidianDistance(double x1, double y1, double x2, double y2);	// returns the euclidian distance between two points
double starSize();	// returns a random val (.05-.5) for star size
bool   isAlien(const Actor* target);	// true if alien
bool   isFriendlyProjectile(const Actor* target);	// true if friendly projectile
bool   isEnemyProjectile(const Actor* target);	// true if enemy projectile
bool   isGoodie(const Actor* target);	// true if goodie

///////////////////////////////////////////////////////////////
// ACTOR INTERFACE
///////////////////////////////////////////////////////////////

class StudentWorld;

class Actor : public GraphObject
{
public:
	// Actors take all parameters of a graph object + a pointer to the world they live in 
	Actor(const int& imageID, const double& startX, const double& startY, 
		  StudentWorld* world, Direction dir, double size, unsigned int depth = 0);
	virtual void doSomething() = 0;	// we never create actor members
	void collide(Actor& other);	// checks if this and other collide, and if so, does the proper action
	
	// helper functions
	void setHealth(int amt);	// health is only relevant for aliens/NB, but it's easier to just define for actors
	int  getHealth() const;
	virtual void takeDamage(int amt);
	int  getActorID() const;	// returns the imageID (so we can tell what type of object each thing is)
	bool isAlive() const;	// true if alive, false if dead
	int  getScore() const;	// returns how many points an actor should give (0 if it flies off the screen)

protected:
	void setScore(int amt);		// useful for knowing when to increase score and when to play sounds
	void kill();	// set alive to dead
	StudentWorld* getWorld() const;

private:
	virtual void collisionProperties(Actor& other) {}	// empty brackets so I don't redefine as empty for star/explosion
	int  m_health;
	int  m_actorID;
	bool m_alive;
	int  m_scorePoints;
	StudentWorld* m_world;
};

///////////////////////////////////////////////////////////////
// STAR INTERFACE
///////////////////////////////////////////////////////////////

class Star : public Actor
{
public:
	Star(double startY, StudentWorld* world);	// constructor for runtime
	Star(double startX, double startY, StudentWorld* world);	// constructor for initialization
	virtual void doSomething();	// move left each tick
};

///////////////////////////////////////////////////////////////
// EXPLOSION INTERFACE
///////////////////////////////////////////////////////////////

class Explosion : public Actor
{
public:
	Explosion(double startX, double startY, StudentWorld* world);
	virtual void doSomething();	// increases in size or goes away each tick
private:
	int m_lifeTime;
	StudentWorld* m_world;
};

///////////////////////////////////////////////////////////////
// NACHENBLASTER INTERFACE
///////////////////////////////////////////////////////////////

class NachenBlaster : public Actor
{
public:
	NachenBlaster(StudentWorld* world);
	virtual void doSomething();	// take user input and 
	
	// helper functions
	void incTorpedoes(int amt);	// public so that goodies can increase our number of torpedoes
	int getCabbageEnergy() const;	// public for the status line to use
	int getNOfTorpedoes() const;	// public for the status line to use

private:
	void moveShip(const int& dir);	// moves the ship in a direction 
	void shootObject(const int& ch);	// shoots if passed SPACEBAR or TAB
	virtual void takeDamage(int amt);	
	virtual void collisionProperties(Actor& other);	// when colliding, calls the other actor's collision properties
	int    m_cabbageEnergy;	
	int    m_nOfTorpedoes;
};

////////////////////////////////////////////////////////////////
// PROJECTILE INTERFACE
////////////////////////////////////////////////////////////////

class Projectile : public Actor
{
public:
	Projectile(int imageID, double startX, double startY, StudentWorld* world, int dir);
	virtual void doSomething();	// projectiles move left/right and maybe rotate each tick
protected:
	virtual void doSomethingProperties(int &dir, int& nOfPixels, bool& doesRotate) = 0; // pass by reference, and then use those values
private:
	char m_source;	// keeps track of who shot the item
};
///////////////////////////////////////////////////////////////
class Cabbage : public Projectile
{
public:
	Cabbage(double startX, double startY, StudentWorld* world);
protected:
	virtual void collisionProperties(Actor& other);
	virtual void doSomethingProperties(int &dir, int& nOfPixels, bool& doesRotate); // sets dir = right, nOfPixels = 8, doesRotate = true
};
///////////////////////////////////////////////////////////////
class Turnip : public Projectile
{
public:
	Turnip(double startX, double startY, StudentWorld* world);
protected:
	virtual void collisionProperties(Actor& other);
	virtual void doSomethingProperties(int &dir, int& nOfPixels, bool& doesRotate); // sets dir = left, nOfPixels = 6, doesRotate = true
};
///////////////////////////////////////////////////////////////
class FTorpedoProjectile : public Projectile
{
public:
	FTorpedoProjectile(double startX, double startY, StudentWorld* world, int dir);
protected:
	virtual void collisionProperties(Actor& other);
	virtual void doSomethingProperties(int &dir, int& nOfPixels, bool& doesRotate); // sets different things depending on whether an alien or user shot it
};

///////////////////////////////////////////////////////////////
// GOODIE INTERFACE
///////////////////////////////////////////////////////////////

class Goodie : public Actor
{
public:
	Goodie(int imageID, double startX, double startY, StudentWorld* world);
	virtual void doSomething();		// goodies move left and down each tick
};
///////////////////////////////////////////////////////////////

class ExtraLife : public Goodie
{
public:
	ExtraLife(double startX, double startY, StudentWorld* world);
private:
	virtual void collisionProperties(Actor& other);	// increases nOfLives
};
///////////////////////////////////////////////////////////////
class Repair : public Goodie
{
public:
	Repair(double startX, double startY, StudentWorld* world);
private:
	virtual void collisionProperties(Actor& other);	// increase health
};
///////////////////////////////////////////////////////////////

class FTorpedoGoodie : public Goodie
{
public:
	FTorpedoGoodie(double startX, double startY, StudentWorld* world);
private:
	virtual void collisionProperties(Actor& other);	// increases 
};

///////////////////////////////////////////////////////////////
// ALIEN INTERFACE
///////////////////////////////////////////////////////////////

class Alien :public Actor
{
public:
	Alien(int imageID, double startY, StudentWorld* world);
	virtual void doSomething();

protected:
	virtual void takeDamage(int amt);
	int    getDirection() const;
	void   setDirection(int dir);
	double getTravelSpeed() const;
	void   setTravelSpeed(double amt);
	void   setFlightPlanLen(int amt);
	void   chooseOtherDirection(int notThisDir);	// choose a random direction that's NOT the direction given

private:
	virtual void collisionProperties(Actor& other);
	virtual void possiblyDropItem() {}	// empty brackets so we don't have to redefine for smallgons
	virtual void shoot();
	virtual void ram() {}		// empty brackets so I don't have to define for smallgons/snagglegons
	virtual void   moveAlien();
	void   chooseRandDirection();	// choose a random direction (down left / left / up left)

	int	   m_dir;
	int    m_flightPlanLen;
	double m_travelSpeed;
};
///////////////////////////////////////////////////////////////
class Smallgon : public Alien
{
public:
	Smallgon(double startY, StudentWorld* world);
};
///////////////////////////////////////////////////////////////
class Smoregon : public Alien
{
public:
	Smoregon(double startY, StudentWorld* world);
private:
	virtual void ram();
	virtual void possiblyDropItem();
};
///////////////////////////////////////////////////////////////
class Snagglegon : public Alien
{
public:
	Snagglegon(double startY, StudentWorld* world);
protected:
	virtual void shoot();
private:
	virtual void collisionProperties(Actor& other);
	virtual void moveAlien();		// snagglegons kinda bounce up and down on the screen
	virtual void possiblyDropItem();
};

#endif // ACTOR_H_
