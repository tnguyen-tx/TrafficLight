# TrafficLight
The project is adopted from UT Austin’s Embedded System Shape the World course. 
The purpose of the project is to solve traffic light issue at a cross section of 2 one-way streets with pedestrian crossing. 
Three sensors are used for each road and pedestrians. A state machine is design to guarantee the traffic flow.
The project runs on Texas Instrument TM4C123 launchpad board.
Output port for road lights is port B (pin 0-5). Output port for pedestrian lights is port E (pin 1 and 3).
Three sensors are 3 pin 0-2 from port F.
A state machine is design to guarantee the traffic flow.
GoE: Cars can go west-east, the light is green
WaitE: Cars on west-east need to wait, the light is yellow
GoN: Cars can go north-south, the light is green
WaitN: Cars on north-south need to wait, the light is yellow
Walk: Pedestrians can walk, walk light is green
Wait_W: Pedestrians need to wait, red light
Hurry: Pedestrians need to be hurry, blinking green light

![image](https://user-images.githubusercontent.com/57820377/168493288-b41eb801-07da-4aab-b961-36d7e69fd83a.png)
