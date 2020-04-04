# 3d-pinball

Physics Pinball

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

## TODO

- * Spring Launcher (Maybe scale one axis independent)
- * Ghost object to lose a life
- * Game over state
- * Diamond to win a life
- * Don't gain points for the same object
- * Sphere 6 DoF

### Documentation y demas cositas

- README
  - Controles de usuario
  - Descripcion de como y en donde cumplimos los requerimientos
  - Instrucciones de compilado y ejecucion
- Funciones
  - JSONLoader::LoadRigidBody
  - *PhysicsUtil (la clase y sus propiedades)
    - *newPhysicsWorld
    - *init
    - *inputListen
    - *collisionListen
    - *launcherUpdate
  - PhysicsDebug (la clase, el hecho que implementa de btIDebugDraw)
    - drawLine
    - y las demas ponerle "no se implementaron" o "se implementaron con funcionalidad vacia"
- Borrar apuntadores
  - theWorld. Lo cual incluye a todas las cosas que le metimos:
    - mCollisions
    - mCollisionDispatcher
    - mOverlappingPairs
    - mConstraints
    - btGhostPairCallback
    - cada constraint dentro del world
  - ghost object (bottomGhost). Incluyendo:
    - Su collisionShape 
  - rigidBody al destruir un objeto. Incluyendo:
    - collisionShape
    - motionState
  - destruir todos los objetos