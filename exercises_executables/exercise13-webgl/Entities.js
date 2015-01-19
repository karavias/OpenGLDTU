"use strict";

//**************************
//* Game State class
//**************************
function State()
{
    this.IsRunning = true;
    this.PlayerHP = PlayerMaxLives;
    this.Score = 0;
}



//**************************
//* EntityBase class
//**************************
function EntityBase(type, pos, radius)
{
    this.Type = type || 0;
    this.Pos = pos || [0, 0, 0];
    this.Radius = radius || 64;
    this.Velocity = [0, 0, 0];
    this.Rotation = 0;
    this.MarkedForDeletion = false;
    this.OverlayColor = [1, 1, 1, 1];
}



//**************************
//* Asteroid class
//**************************
function Asteroid(type, pos, radius, velocity, rotVelocity)
{
    EntityBase.call(this, type, pos, radius);

    this.Velocity = velocity || [0, 0, 0];
    this.RotationalVelocity = rotVelocity || 0;
}

Asteroid.prototype = Object.create(EntityBase.prototype);
Asteroid.prototype.constructor = Asteroid;

//updates asteroid, elapsed is the time in ms since last update
Asteroid.prototype.Update = function(elapsed)
{
    //TODO: update position, p += v*t
    this.Pos[0] += this.Velocity[0] * elapsed;
    this.Pos[1] += this.Velocity[1] * elapsed;

    //TODO: update rotation, r += rv*t
    this.Rotation += this.RotationalVelocity * elapsed;

    //check if out of bounds and mark for deletion if true
    if (OutOfBounds(this))
        this.MarkedForDeletion = true;
};



//**************************
//* Shot class
//**************************
function Shot(type, pos, radius, velocity)
{
    EntityBase.call(this, type, pos, radius);

    this.Velocity = velocity || [0, 0, 0];
}

Shot.prototype = Object.create(EntityBase.prototype);
Shot.prototype.constructor = Shot;

//updates shot
Shot.prototype.Update = function (elapsed)
{
    //update position
    this.Pos[0] += elapsed * this.Velocity[0];
    this.Pos[1] += elapsed * this.Velocity[1];

    //check if out of bounds and mark for deletion if true
    if (OutOfBounds(this))
        this.MarkedForDeletion = true;
};



//**************************
//* Ship class
//**************************
function Ship(pos)
{
    EntityBase.call(this, 0, pos, 32);

    this.Direction = [0, 1, 0];
    this.IsThrusting = false;
}

Ship.prototype = Object.create(EntityBase.prototype);
Ship.prototype.constructor = Ship;

//updates ship, elapsed is the time in ms since last update
Ship.prototype.Update = function(elapsed)
{
    //update ship position, p += v*t
    this.Pos[0] += elapsed * this.Velocity[0];
    this.Pos[1] += elapsed * this.Velocity[1];

    //update velocity (deaccelerate) to make the ship stop slowly
    this.Velocity[0] *= 0.99;
    this.Velocity[1] *= 0.99;

    //clamp velocity to zero if it is close to zero (prevents floating point drifting)
    if (Math.abs(this.Velocity[0]) < 0.01) this.Velocity[0] = 0;
    if (Math.abs(this.Velocity[1]) < 0.01) this.Velocity[1] = 0;
};

//change shop rotation and direction to at (x,y)
Ship.prototype.LookAt = function(x, y)
{
    
    var cY = canvas.height - y;
    
    //compute direction from mouse x,y (note: mouse y is from top to bottom, canvas y is from bottom to top)
    this.Direction = [x - this.Pos[0], cY - this.Pos[1], 0];
    
    //normalize direction
    this.Direction = normalize(this.Direction);
    
    //compute rotation from direction (in degrees), add this to the -90 degrees
    this.Rotation = -180 - Math.atan2(this.Direction[0], this.Direction[1]) * 180 / Math.PI;
};

//fires a shot in the direction the ship is facing
Ship.prototype.Shoot = function()
{

    //TODO: compute shot spawn position, hint: use ship position, direction and radius to spawn the shot just outside the ship in the direction it is facing
    var pos = [this.Pos[0], this.Pos[1], 0];


    //set shot velocity in the direction of the ship, choose appropriate speed factor < 1
    var vel = [this.Direction[0] * 0.5, this.Direction[1] * 0.5, 0];


    //create new shot (a scaled and colored asteroid)
    var shot = new Shot(4, pos, 5, vel); //we use sprite type 4 (asteroid)
    shot.OverlayColor = [1, 0.5, 0.5, 1];

    //add shot to shots list
    Shots.push(shot);
};