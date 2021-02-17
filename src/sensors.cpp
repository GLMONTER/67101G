#include"sensors.hpp"

//Sensor init
pros::Optical vSensor(8);
pros::Distance distance_sensor(18);

//bool that holds the state of the limiter
extern bool canLimit;
//define the alliance color to sort the correct ball color.
#define BLUE

//tuning variables for the different robots.
static int32_t delayEject = 850;
static int32_t bottomSpeed = 100;

int32_t topVelocity = 400;
static int32_t minVelocity =350;


//enable/disable sorting task
bool SORT_SYS_ENABLE = true;

unsigned int limitPresses = 0;

//polls limit switch to check for when a ball passes through.
void pollSensors()
{
	while(true)
	{
		while(distance_sensor.get() > 50)
		{
			pros::delay(10);
			continue;
		}
		while(distance_sensor.get() < 50)
		{
			pros::delay(10);
			continue;
		}
		limitPresses++;
		pros::lcd::print(4, "%d", limitPresses);
		/*
		if(topLimit.get_new_press())
		{
			limitPresses++;
			std::cout<<limitPresses<<std::endl;
			std::cout<<"new press!"<<std::endl;
		}
		pros::delay(10);
		*/
	}
}
bool disableTop = false;
bool disableBottom = false;
extern bool runningAuton;
//this function will sort the balls based on the color signature passed in.
//The task will start at the beginning of the program with the correct ball color to start.
void sort()
{
	//set loader brake modes to lock so the alliance ball can stop at the top of the loader
	topSystem.set_brake_mode(pros::E_MOTOR_BRAKE_BRAKE);
	bottomSystem.set_brake_mode(pros::E_MOTOR_BRAKE_BRAKE);
	//turn on optical LED
	vSensor.set_led_pwm(100);
	while(true)
	{
		if(canLimit)
		{
			controller.rumble(".");
		}
		if(disableBottom && disableTop)
		{
			topSystem.move(0);
			bottomSystem.move(0);
		}
		vSensor.set_led_pwm(100);

        //if the sorting system is disabled then don't attemp to sort.
        if(!SORT_SYS_ENABLE)
            continue;

		if(runningAuton)
		{
			static bool runSwitch = false;

			//if the top limiter sensor is hit and the program is allowed to limit, stop loading more.
			if(distance_sensor.get() < 50 && canLimit && !runSwitch)
			{
				topSystem.move_velocity(0);
				if(!disableBottom)
					bottomSystem.move(-127);
				pros::delay(50);
				bottomSystem.move_velocity(0);
				if(!disableTop)
					topSystem.move_velocity(topVelocity);
				runSwitch = true;
				std::cout<<"LIMIT"<<std::endl;
				continue;
			}

			if(distance_sensor.get() < 50 && canLimit && runSwitch)
			{
				bottomSystem.move_velocity(0);
				pros::delay(10);
				continue;
			}

			if(distance_sensor.get() > 50 && canLimit && runSwitch)
			{
				runSwitch = false;
			}
		}
		else
		{
			if(distance_sensor.get() < 35 && canLimit)
			{
				bottomSystem.move_velocity(0);
				topSystem.move_velocity(0);
				pros::delay(10);
				continue;
			}
		}
		//if the alliance color ball was found the just load up
		
		if((vSensor.get_rgb().red / vSensor.get_rgb().blue) > 2)
		{
			if(std::abs(topSystem.get_actual_velocity()) > minVelocity)
			{
				#ifdef RED
				if(!disableTop)
					topSystem.move_velocity(topVelocity);
				if(!disableBottom)
					bottomSystem.move(bottomSpeed);
				#else
				if(!disableTop)
					topSystem.move_velocity(-topVelocity);
				if(!disableBottom)
					bottomSystem.move(bottomSpeed);
				pros::delay(delayEject);
				#endif
			}
			else
			{
				#ifdef RED
				if(!disableTop)
					topSystem.move_velocity(topVelocity);
				#else
				if(!disableTop)
					topSystem.move_velocity(-topVelocity);
				#endif
				/*
				if(!disableBottom)
					bottomSystem.move(bottomSpeed);
					*/
			}
		}
		//if the alliance ball is not detected then search for the enemy ball for discarding.
		else
		if((vSensor.get_rgb().blue / vSensor.get_rgb().red) > 1.6)
		{
			if(std::abs(topSystem.get_actual_velocity()) > minVelocity)
			{
			#ifdef RED
			if(!disableTop)
				topSystem.move_velocity(-topVelocity);
			if(!disableBottom)
				bottomSystem.move(bottomSpeed);
			pros::delay(delayEject);
			#else
			
			if(!disableTop)
				topSystem.move_velocity(topVelocity);
			if(!disableBottom)
				bottomSystem.move(bottomSpeed);
			#endif
			}
			else
			{
				#ifdef RED
				if(!disableTop)
					topSystem.move_velocity(-topVelocity);
				#else
				if(!disableTop)
					topSystem.move_velocity(topVelocity);
				#endif
				/*
				if(!disableBottom)
					bottomSystem.move(bottomSpeed);
					*/
			}
		}
		//if nothing was found then just load like normal
		else
		{	
			if(!disableTop)
				topSystem.move_velocity(topVelocity);
			
			if(!disableBottom)
				bottomSystem.move(bottomSpeed);
		}
		vSensor.set_led_pwm(100);

		//make the thread sleep to prevent other threads from being starved of resources.
		pros::Task::delay(10);
	}
}
