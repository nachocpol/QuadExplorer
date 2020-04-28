# Quad Explorer

This is my attempt to create a standalone quad copter, from the hardware to the software that makes it fly. This project is divided in several parts:

### QuadExplorerApp
This app simulates the drone flying. It uses my own 3D rendering framework and PhysicsX to simulate the behaviour of the quad. This software is useful to test the flight controller without deploying it to the board. It can also be used to  tweak the gains for the different PID controllers.

### Board
Software that runs on the quadcopter hardware. This implements basic things like sensor reading, noise removal and BT/Serial conections.

### ControllerApp
Unity project to control the drone from a phone. To build this from source, you need the "Bluetooth LE for iOS, tvOS and Android" library.