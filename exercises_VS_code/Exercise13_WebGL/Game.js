"use strict";

//**************************
//* WebGL
//**************************
var gl;
var canvas;

//**************************
//* Buffers, Shaders
//**************************
var quadVertexBuffer;
var quadVertexElementArrayBuffer;
var spriteShader;
var starfieldShader;

//**************************
//* HTML Elements
//**************************
var frametimeElem;
var astCountElem;
var shtCountElem;
var scoreElem;
var lifeElem;
var gameoverElem;
var particleStatsElem;

//**************************
//* Projection, View
//**************************
var projection;
var view;

//**************************
//* Misc
//**************************
var RAD2DEGREE = 57.295779513;
var CanvasRect;
var tickTimeLast = performance.now();
var frame = 0;
var AsteroidSpawnTimer = 0;

//**************************
//* Game objects
//**************************
var GameState;
var ParticleSystem;
var Player;
var Starfield;
var Asteroids = [];
var Shots = [];

//**************************
//* Settings
//**************************
var AsteroidMinRadius = 15;
var AsteroidMaxRadius = 50;
var AsteroidSpawnInterval = 2000; //ms
var PlayerMaxLives = 4;
var PlayerThrustSpeed = 0.12;





//**************************
//* Initialize
//**************************
window.onload = function init()
{
    //Get Canvas
    canvas = document.getElementById("gl-canvas");
    CanvasRect = canvas.getBoundingClientRect();

    //html elements
    frametimeElem = document.getElementById("frametime");
    astCountElem = document.getElementById("astCount");
    shtCountElem = document.getElementById("shtCount");
    scoreElem = document.getElementById("score");
    lifeElem = document.getElementById("life");
    gameoverElem = document.getElementById("gameover");
    particleStatsElem = document.getElementById("particleStats");
    


    //Get WebGL Context
    gl = WebGLUtils.setupWebGL(canvas, { alpha:false });
    if (!gl) {
        alert("Failed to get context!");
        return;
    }

    //Setup viewport and clear color
    gl.viewport(0, 0, canvas.width, canvas.height);
    gl.clearColor(0,0,0, 1.0);
    gl.clear(gl.COLOR_BUFFER_BIT);

    //setup view and projection matrices
    projection = ortho(0, canvas.width, 0, canvas.height, -1, 2); //left, right, bottom, top, near, far
    view = lookAt(vec3(0, 0, 1), vec3(0, 0, 0), vec3(0, 1, 0));

    //Load, compile and link the shaders
    spriteShader = initShaders(gl, "sprite-vert", "sprite-frag");
    starfieldShader = initShaders(gl, "starfield-vert", "starfield-frag");
    //gl.useProgram(spriteShader);

    //load interleaved vertex buffer
    createQuadVertexBufferObject();

    //load spritesheet texture
    loadTextures();

    //setup keyboard and mouse event handlers
    initKeyboard();
    initMouse();



    //init game state
    GameState = new State();
    updateGUI();


    //initialize player
    Player = new Ship([canvas.width / 2, canvas.height / 2, 0]);


    //spawn initial asteroids
    SpawnAsteroids(5, [0,0], [canvas.width, canvas.height]);


    //init particle system
    ParticleSystem = new ParticleSystem();
    ParticleSystem.Initialize(200);


    //init background starfield
    Starfield = new Starfield();



    //start game loop
    tick();
}


//**************************
//* Game Tick
//**************************
function tick()
{
    //elapsed time since last frame and frame count
    var frameTime = performance.now();
    var elapsed = frameTime - tickTimeLast;
    frame += 1;

    //update game logic
    update(elapsed);

    //draw
    draw(elapsed);


    //debug stuff
    frametimeElem.innerHTML = "FrameTime: " + (performance.now() - frameTime).toPrecision(4) + "ms";
    astCountElem.innerHTML = "Asteroid Count: " + Asteroids.length;
    shtCountElem.innerHTML = "Shot Count: " + Shots.length;
    particleStatsElem.innerHTML = "Particle System: " + ParticleSystem.Particles.length + " particles active, " + ParticleSystem.ParticlePool.length + " left in pool";



    tickTimeLast = performance.now();

    requestAnimFrame(tick);
}


//**************************
//* Update
//**************************
function update(elapsed)
{

    //update player
    if (GameState.IsRunning === true)
    {
        Player.Update(elapsed);
        HandlePlayerAsteroidCollisions();
    }



    //update asteroids
    for (var i = 0; i < Asteroids.length; i++)
    {
        Asteroids[i].Update(elapsed);
    }

    //check for asteroid vs asteroid collisions
    HandleAsteroidAsteroidCollisions();

    //remove asteroids that are marked for deletion
    Asteroids = Asteroids.filter(function (ast) { return ast.MarkedForDeletion === false; });

    //spawn new asteroids
    AsteroidSpawnTimer += elapsed;
    if (AsteroidSpawnTimer >= AsteroidSpawnInterval)
    {
        AsteroidSpawnTimer = 0;
        SpawnAsteroids(5, [canvas.width + 32, -64], [canvas.width + 128, canvas.height+64]);
    }



    //update shots
    for (var i = 0; i < Shots.length; i++)
    {
        Shots[i].Update(elapsed);
    }

    //remove shots that are marked for deletion
    Shots = Shots.filter(function (sht) { return sht.MarkedForDeletion === false; });

    //check for shot vs asteroid collisions
    HandleShotAsteroidCollisions();



    //update particles
    ParticleSystem.Update(elapsed);
}


//**************************
//* Draw
//**************************
function draw(elapsed)
{
    //setup OpenGL render state
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
    gl.enable(gl.BLEND);
    gl.disable(gl.DEPTH_TEST);
    gl.blendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA);

    var model, modelView;
    
    //bind vertex and index buffers
    gl.bindBuffer(gl.ARRAY_BUFFER, quadVertexBuffer);
    bindAttributes();
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, quadVertexElementArrayBuffer);



    //draw starfield background
    Starfield.Draw(elapsed);
    

    //setup common uniforms and sprite shader, used by asteroids, shots, player ship and particles
    gl.useProgram(spriteShader);
    gl.uniformMatrix4fv(gl.getUniformLocation(spriteShader, "Projection"), false, flatten(projection));
    gl.uniform1i(gl.getUniformLocation(spriteShader, "tex"), 0);
    gl.uniform2fv(gl.getUniformLocation(spriteShader, "SheetSize"), [512, 512]);
    gl.uniform2fv(gl.getUniformLocation(spriteShader, "SpriteSize"), [128, 128]);






    //draw all asteroids
    for (var i = 0; i < Asteroids.length; i++)
    {
        model = mult(translate(Asteroids[i].Pos), mult(scale(Asteroids[i].Radius*2, Asteroids[i].Radius*2, Asteroids[i].Radius*2), rotate(Asteroids[i].Rotation, vec3(0, 0, 1))));
        modelView = mult(view, model);
        gl.uniformMatrix4fv(gl.getUniformLocation(spriteShader, "ModelView"), false, flatten(modelView));
        gl.uniform4fv(gl.getUniformLocation(spriteShader, "OverlayColor"), Asteroids[i].OverlayColor);
        gl.uniform1fv(gl.getUniformLocation(spriteShader, "SpriteIndex"), [Asteroids[i].Type]);
        gl.drawElements(gl.TRIANGLES, 6, gl.UNSIGNED_SHORT, 0);
    }


    //draw all shots
    for (var i = 0; i < Shots.length; i++)
    {
        model = mult(translate(Shots[i].Pos), mult(scale(Shots[i].Radius * 2, Shots[i].Radius * 2, Shots[i].Radius * 2), rotate(Shots[i].Rotation, vec3(0, 0, 1))));
        modelView = mult(view, model);
        gl.uniformMatrix4fv(gl.getUniformLocation(spriteShader, "ModelView"), false, flatten(modelView));
        gl.uniform4fv(gl.getUniformLocation(spriteShader, "OverlayColor"), Shots[i].OverlayColor);
        gl.uniform1fv(gl.getUniformLocation(spriteShader, "SpriteIndex"), [Shots[i].Type]);
        gl.drawElements(gl.TRIANGLES, 6, gl.UNSIGNED_SHORT, 0);
    }


    //draw player
    if (GameState.IsRunning)
    {
        model = mult(translate(Player.Pos), mult(scale(Player.Radius * 2, Player.Radius * 2, Player.Radius * 2), rotate(Player.Rotation, vec3(0, 0, 1))));
        modelView = mult(view, model);
        gl.uniformMatrix4fv(gl.getUniformLocation(spriteShader, "ModelView"), false, flatten(modelView));
        gl.uniform4fv(gl.getUniformLocation(spriteShader, "OverlayColor"), Player.OverlayColor);
        gl.uniform1fv(gl.getUniformLocation(spriteShader, "SpriteIndex"), [0]);
        gl.drawElements(gl.TRIANGLES, 6, gl.UNSIGNED_SHORT, 0);

        if (Player.IsThrusting)
        {
            //draw engine trail sprite when the player is thrusting (pressing 'W')
            gl.uniform1fv(gl.getUniformLocation(spriteShader, "SpriteIndex"), [1]);
            gl.drawElements(gl.TRIANGLES, 6, gl.UNSIGNED_SHORT, 0);
        }
    }


    //draw particles
    ParticleSystem.Draw(elapsed);
}


//***********************************
//* Atrributes, buffers and texture
//***********************************
function bindAttributes()
{
    var sizeOfFloat = 4;
    var vertexLength = 5 * sizeOfFloat;

    var vertexLocation = gl.getAttribLocation(spriteShader, "position");
    gl.enableVertexAttribArray(vertexLocation);
    gl.vertexAttribPointer(vertexLocation, 3, gl.FLOAT, false, vertexLength, 0);

    var uvLocation = gl.getAttribLocation(spriteShader, "uv");
    gl.enableVertexAttribArray(uvLocation);
    gl.vertexAttribPointer(uvLocation, 2, gl.FLOAT, false, vertexLength, 3 * sizeOfFloat);
}

function createQuadVertexBufferObject()
{
    // create vertex and index buffer
    var vertexCount = 4;
    var vertexSize = 3 + 2;
    var vertices = new Float32Array(vertexCount * vertexSize); // position, uv

    //TODO: create the vertices of unit quad with center in 0,0,0
    //the order should be top-left, top-right, bottom-left, bottom-right
    var vertexData = [
        -0.5, 0.5, 0,
        0.5, 0.5, 0,
        -0.5, -0.5, 0,
        0.5, -0.5, 0

    ];

    //TODO: create the texture coordinates matching the vertices from above
    var uvData = [
        0, 0,
        1, 0,
        0, 1,
        1, 1
    ];



    //indices
    var indices = [
        0, 1, 2,
        1, 2, 3
    ];

    //interleave vertex data
    for (var i = 0; i < vertexCount; i++)
    {
        vertices[i * vertexSize    ] = vertexData[i * 3    ];
        vertices[i * vertexSize + 1] = vertexData[i * 3 + 1];
        vertices[i * vertexSize + 2] = vertexData[i * 3 + 2];
        vertices[i * vertexSize + 3] = uvData[i * 2];
        vertices[i * vertexSize + 4] = uvData[i * 2 + 1];
    }

    //create and bind vertex buffer and upload vertices data
    quadVertexBuffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, quadVertexBuffer);
    gl.bufferData(gl.ARRAY_BUFFER, vertices, gl.STATIC_DRAW);

    //create and bind index buffer and upload indices data
    quadVertexElementArrayBuffer = gl.createBuffer();
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, quadVertexElementArrayBuffer);
    gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, new Uint16Array(indices), gl.STATIC_DRAW);
}

function loadTextures()
{
    var image = new Image();
    image.onload = function ()
    {
        //create texture id, activate and bind
        var texID = gl.createTexture();
        gl.activeTexture(gl.TEXTURE0);
        gl.bindTexture(gl.TEXTURE_2D, texID);

        //set texture data
        gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, true);
        gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, image);

        //set texture parameters
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
    };

    image.src = imageSheet; //change to 'imageSheet' when using the texture atlas in part 2
}


//****************************
//* Keyboard, mouse and GUI
//****************************
function initKeyboard()
{
    document.onkeyup = function(event) 
    {
        //restart game when 'space' is pressed
        if (String.fromCharCode(event.keyCode) == ' ')
        {
            GameState = new State();
            updateGUI();

            //center player and reset velocity
            Player.Pos = [canvas.width / 2, canvas.height / 2, 0];
            Player.Velocity = [0, 0, 0];
        }

        //disable player thrusting when a key is relased
        Player.IsThrusting = false;
    };

    document.onkeydown = function(event)
    {
        //move player when 'W' is down
        if (String.fromCharCode(event.keyCode) == 'W')
        {
            //set player velocity
            Player.Velocity = [Player.Direction[0] * PlayerThrustSpeed, Player.Direction[1] * PlayerThrustSpeed, 0];

            //enable player thrusting
            Player.IsThrusting = true;
        }
    };
}

function initMouse()
{
    document.onmousemove = function(e)
    {
        //change player LookAt when game is running
        if (GameState.IsRunning)
            Player.LookAt(e.clientX, e.clientY);
    };

    document.onmousedown = function(event)
    {
        //fire shot if game is running
        if (GameState.IsRunning)
            Player.Shoot();

        //always return false, prevents 'character selection' cursor from showing when click-dragging
        return false;
    };
}

function updateGUI()
{
    //update score and life
    scoreElem.innerHTML = "SCORE : " + GameState.Score;
    lifeElem.innerHTML = GameState.PlayerHP + " : LIVES";

    //hide or show game over message
    if (GameState.IsRunning)
        gameoverElem.style.display = 'none';
    else
        gameoverElem.style.display = '';
}


//**************************
//* Spawn asteroid wave
//**************************
function SpawnAsteroids(iNum, vMin, vMax)
{
    var iCount = 0; //how many successfully spawned
    var stop = 0; //stop limit, break out if needed

    //loop to spawn iNum asteroids
    while (iCount < iNum && stop < 200)
    {
        //random settings for new asteroids
        var type = Math.floor(Math.random() * (15 - 4 + 1)) + 4;                                    //type (texture sprite index 4-15 for asteroids)
        var x = Math.floor(Math.random() * (vMax[0] - vMin[0] + 1) + vMin[0]);                      //position x
        var y = Math.floor(Math.random() * (vMax[1] - vMin[1] + 1) + vMin[1]);                      //position y
        var vx = -Math.random() * 0.16;                                                             //velocity x
        var vy = (Math.random() * 2 - 1) * 0.06;                                                    //velocity y
        var r = AsteroidMinRadius + Math.random() * (AsteroidMaxRadius - AsteroidMinRadius) + 1;    //radius (and mass)
        var rv = 0.06 * (Math.random() * 2 - 1) * AsteroidMinRadius / r;                            //rotational velocity

        //create new asteroid
        var ast = new Asteroid(type, [x, y, 0], r, [vx, vy, 0], rv);

        //create tmp player (asteroid) to use for bounding check agains new asteroid (to prevent asteroids from spawning on top off player)
        var tmpPlayer = new Asteroid(0, Player.Pos, Player.Radius * 3);

        //if the new asteroid isn't colliding with any existing asteroids or the player
        if (CheckAsteroidCollisionsSingleVsAll(ast, 0) === -1 && AsteroidCollision(ast, tmpPlayer) === false)
        {
            Asteroids.push(ast);
            iCount++;
        }

        stop++;
    }
}