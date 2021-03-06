#include"main.h"

pros::Imu imu(21);

extern bool SORT_SYS_ENABLE;
extern bool canLimit;
extern unsigned int limitPresses;
extern bool disableTop;
extern bool disableBottom;
extern int32_t topVelocity;

enum loaderSetting
{
    Forward = 0,
    Backward = 1,
    Disabled = 2
};
bool runningAuton = false;
//wait until a certain number of balls have gone through
static void waitUntilPressCount(const unsigned int pressCount, const bool waitUntilHold)
{
    static bool printed = false;

    std::cout<<"started wait"<<std::endl;
    std::cout<<pressCount<<std::endl;
    std::cout<<limitPresses<<std::endl;
    canLimit = false;
    while(limitPresses < pressCount)
    {
        std::cout<<limitPresses<<std::endl;
        canLimit = false;
        if(!printed)
        {
            std::cout<<"waiting on limits"<<std::endl;
            printed = true;
        }

        continue;
    }
    if(waitUntilHold)
    {
        std::cout<<"starting wait until hold"<<std::endl;
        while(distance_sensor.get() < 50)
        {
            continue;
        }
        while(distance_sensor.get() > 50)
        {
            continue;
        }
        std::cout<<"finished"<<std::endl;
    }
    canLimit = true;
}

//turn using gyro and PID
static void gyroTurn(const float deg)
{
    leftBack.set_brake_mode(pros::E_MOTOR_BRAKE_BRAKE);
    leftFront.set_brake_mode(pros::E_MOTOR_BRAKE_BRAKE);
    rightBack.set_brake_mode(pros::E_MOTOR_BRAKE_BRAKE);
    rightFront.set_brake_mode(pros::E_MOTOR_BRAKE_BRAKE);


    float error = 10.0;
    float integral = 0.0;
    float derivative = 0.0;
    float perror = 0.0;
    float value = 0.0;

    float target = deg;
    float Ki = -0.0015;
    float Kd = -0.5;
    float Kp = -7.25;

    while (abs(error) > 1 || leftBack.get_actual_velocity() > 0.1)
    {
        pros::lcd::print(0, "val: %f\n", imu.get_yaw());
        error =  target - imu.get_yaw();
        printf("%f \n", error);
        integral = integral + error;
        if (abs(error) < 2)
        {
            integral = 0.0;
        }
        derivative = error - perror;
        perror = error;
        value = (integral*Ki) + (derivative*Kd) + (error*Kp);
        setDrive(-value, value);


        pros::delay(5);
    }
    setDrive(0,0);
}


static auto chassis = ChassisControllerBuilder()
    .withMotors
    (
        20,  // Top left
        11, // Top right (reversed)
        1, // Bottom right (reversed)
        9   // Bottom left
    )
    .withGains(
        {0.0025, 0.0005, 0.0001}, // Distance controller gains
        {0.0025, 0, 0.0001}, // Turn controller gains
        {0.002, 0.001, 0.0001})  // Angle controller gains (helps drive straight)

    .withDimensions(AbstractMotor::gearset::green, {{4_in, 11.5_in}, imev5GreenTPR})
    .withOdometry()
    .buildOdometry();

auto xModel = std::dynamic_pointer_cast<XDriveModel>(chassis->getModel());

static void strafeAbstract(std::shared_ptr<okapi::XDriveModel>& model, double velocityPower, const uint32_t timeToStrafe, const uint32_t timeToSettle)
{
    model->strafe(velocityPower);
    pros::delay(timeToStrafe);
    model->stop();
    pros::delay(timeToSettle);
}
static void swingTurn(const int32_t forwardPower, const int32_t turnPower, const uint32_t timeToRun, const uint32_t driveSettle, const bool settle)
{
  setDrive(forwardPower + turnPower, forwardPower - turnPower);
  pros::Task::delay(timeToRun);
  setDrive(0,0);
  if(settle)
  {
      pros::Task::delay(driveSettle);
  }
}

void rightSide()
{

  chassis->setMaxVelocity(130);

  topSystem.move(127);
  pros::delay(500);
  topSystem.move(0);
topVelocity = 390;
  //start lifts and sorting
  setLoaders(loaderSetting::Backward);
  //swingTurn(80, -15,1500,450,true);
  swingTurn(80, 18,675,0,true);
  setLoaders(loaderSetting::Forward);
  swingTurn(80, 18,675,450,true);
  waitUntilPressCount(1, false);


  pros::delay(250);
  topVelocity = 400;
  chassis->moveDistance(-1_ft);
  disableTop = true;
  disableBottom = true;
  setLoaders(loaderSetting::Disabled);

}
void leftSide()
{

  chassis->setMaxVelocity(130);

  topSystem.move(127);
  pros::delay(500);
  topSystem.move(0);

  //start lifts and sorting


  setLoaders(loaderSetting::Backward);
  //swingTurn(80, -15,1500,450,true);
  swingTurn(80, -15,750,0,true);
  setLoaders(loaderSetting::Forward);
  swingTurn(80, -15,750,450,true);
  waitUntilPressCount(2, true);
  disableTop = true;
  disableBottom = true;

  setLoaders(loaderSetting::Backward);
  pros::delay(500);
  chassis->moveDistance(-1_ft);
}
void runAuton()
{
  runningAuton = true;
  //init gyro
  imu.reset();
  pros::delay(2000);
rightSide();



  runningAuton = false;
}
