# 3d-pinball

Physics based Pinball machine.

José Daniel Casado Aguilar & David Zuñiga Navarro

## User controls

By holding the `M` key, the "launcher" box will start charge and move down.
By releasing the key, the launcher will launch itself and the ball. The force of the launch will depend on how much time the key was pressed.

By pressing the `left` or `right` arrows, the left or right paddles will turn up respectively.

## Project overview

We used Bullet for simulating movement, forces and collision detection and responses.

In the following I'll describe the classes and functions that are involved in physics simulation:

### GDP2019-20/JSONLoader

#### LoadRigidBody

This function receives a `json` object with all of the physics specifications: shape, measurements, mass, etc.

It creates a `btRigidBody` and returns it.

#### JSONLoadGameObjects

This reads a `json` defining a game object. We added the functionality to call `LoadRigidBody` in case the `json` has a `Physics` property.

If it's created, it adds it to the physics world, attaches it to the game object, and sets the *user pointer* of the `btRigidBody` to the `cGameObject`.

We also detect if the json specification has the "givesPoint" flag. If they do, we add that object to a special point-giver list.

We support `btBoxShape`, `btSphereShape`, `btCylinderShapeZ` and `btConvexHullShape`

Additioanlly. we also set collisions groups and masks. The ball is on group 1 and collides with mask 2 and everything else is on group 2 and collides with mask 1.
This way, the only collisions we process are the ones between the ball and the other collision bodies.

### GDP2019-20/cGameObject

This class represents all of the objects that are in the scene. We added a `btRigidBody` pointer to it so we can reference the physics info and functionality when needed, like when drawing the object.

### GDP2019-20/PhysicsUtil

This class stores the `btDiscreteDynamicsWorld` we use to store all of the rigid bodies and simulate the physics.

We have also pointer to relevant objects in the scene: the paddles, the ball, the launcher, etc.

It also provides functions to create, read and interact with this world.

#### init

In this function we set up more specific properties of special rigid bodies that are not defined in the json specfication.

In here we set up the `btHingeConstraint`s for the paddles, `btSliderConstraint` for the launcher and the `btGeneric6DofConstraint` for the ball.

We also create a `btPairCachingGhostObject` for the area on the bottom of the pinball machine.

#### paddleInputListen

In this function we detect the state of the `left` and `right` keys and add different rotational velocities to the paddle rigid bodies depedning of the state of the key.

#### collisionListen

In here, we go through the detected collisions in this frame in the physics world. We check if any of the bodies involved in the collision are in the list of special point-giver objects. If they are, we add points to the score.

We also have a special object (with the name "diamond") that, instead of giving points, gives you an extra life.

Lastly, in this function we also go through the overlapping objects to the ghost body at the bottom of the machine. If any of those objects are the ball, we sustract one life to the score and reset the ball position.

#### launcherUpdate

In here we listen to the state of the `M` key to charge up and release the launcher according to how much time the key was pressed. There's a maximum of 5 second of charge. More time will not increase the final force of the launcher.

### GDP2019_20/PhysicsDebug

Implementation of `btIDebugDraw`. It tells bullet how to render things on screen so we can see the shape of the collision shapes.

## Compile and run instructions

To build, open `j_casadoaguilar_engine.sln` with Visual Studio. Select `Release` and `x64` and then `Build Solution` (Ctrl + Shift + B).

To run, within Visual Studio, click on `Local Windows Debugger` (F5)

## JSON Format

### gameObjects.json

```json
{
    ...
    "Physics":
    {
        "shape": "sphere" | "box" | "cylinder" | "mesh",
        // if not specified, a rigid body will not be created
        "mass": 10.0, // optional, default to 0 (static)
        // for sphere and cylinder
        "radius": 3.0,
        // for box 
        "height": 10.0, // full height (Y axis)
        "width": 8.0, // full width (X axis)
        // for box and cylinder (z oriented cylinder)
        "depth": 2.0, // full depth (Z axis)
        // the mesh shape will use the mesh data (maybe)
        // The rigid body starting position and rotation
        // will be the same as the game object

        // how much energy the body will lose (or earn)
        // with each bounce
        "restitution": 0.8
    },
    ...
    // if colliding with this object gives the player points
    "givesPoints": true
}
```
