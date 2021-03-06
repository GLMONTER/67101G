#include"control_sys.hpp"

pros::Controller controller(pros::E_CONTROLLER_MASTER);

pros::Motor leftLoader(12, pros::E_MOTOR_GEARSET_06, false);
pros::Motor rightLoader(4, pros::E_MOTOR_GEARSET_06, true);

pros::Motor rightFront(11, true);
pros::Motor leftFront(20, false);
pros::Motor rightBack(1, true);
pros::Motor leftBack(9, false);

//includes flywheel, and the two lifts
pros::Motor topSystem(5, pros::E_MOTOR_GEARSET_06, false);

pros::Motor bottomSystem(19, pros::E_MOTOR_GEARSET_06, false);


//toggles for lift
static bool buttonToggleR = 0;
static bool buttonPressedR = 0;

static bool buttonToggleF = 0;
static bool buttonPressedF = 0;

extern bool sortToggle;
enum loaderSetting
{
    Forward = 0,
    Backward = 1,
    Disabled = 2
};
void setDrive(const int32_t leftPower, const int32_t rightPower)
{
    rightFront.move(rightPower);
    rightBack.move(rightPower);
    leftFront.move(leftPower);
    leftBack.move(leftPower);
}

void setLoaders(const int setting)
{
    if(setting == loaderSetting::Forward)
    {
        leftLoader.move(127);
        rightLoader.move(127);
    }
    else if(setting == loaderSetting::Backward)
    {
        leftLoader.move(-127);
        rightLoader.move(-127); 
    }
    else if(setting == loaderSetting::Disabled)
    {
        leftLoader.move(0);
        rightLoader.move(0);  
    }
}

void sortFailsafe()
{
    //go forward with drum
    if(controller.get_digital(pros::E_CONTROLLER_DIGITAL_A))
    {
        //if the forward button toggle isn't on then continute
        if(!buttonPressedF)
        {
            //actaully flip the toggle, this is why the type has to be int
            buttonToggleF = 1 - buttonToggleF;
            //changed button pressed to true
            buttonPressedF = 1;
            //change the backward toggle to false so we don't try to go backwards and forwards
            buttonToggleR = false;
        }
    }
    //switch back to normal buttton state but leave toggle on if button isn't pressed.
    else
        buttonPressedF = 0;

    //if our forward toggle is on, then eat the balls :D
    if(buttonToggleF == true)
    {
        topSystem.move(-127);
		bottomSystem.move(-127);
    }
    //check if other toggle is on if we need to really stop the motor
    else
    {
        if(!buttonToggleR && !buttonToggleF)
        {
            topSystem.move(-127);
			bottomSystem.move(-127);
        }
    }
    //go backwards with drum
    if(controller.get_digital(pros::E_CONTROLLER_DIGITAL_Y))
    {
        //if we haven't pressed the button then toggle the button
        if(!buttonPressedR)
        {
            //actually toggle the button, this is why the type is int
            buttonToggleR = 1 - buttonToggleR;

            buttonPressedR = 1;

            //so we stop going forward.
            buttonToggleF = false;
        }
    }
    //else, then turn button pressed to false
    else
        buttonPressedR = 0;

    //if backward button toggle is on, then start the motor backward
    if(buttonToggleR == true)
    {
        topSystem.move(127);
		bottomSystem.move(127);
    }
    //else, check if the forward toggle is off, then stop.
    else
    {
        if(!buttonToggleF && !buttonToggleR)
        {
            topSystem.move(0);
		    bottomSystem.move(0);
        }
    }
}