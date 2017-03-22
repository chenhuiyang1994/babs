// Shamelessly based ona CWRU EECS class package's code (sine commander) and heavily modified, courtesy Dr. Wyatt Newman
// Last updated Mar 22 2017 by Trent Ziemer
#include <ros/ros.h>
#include <std_msgs/Int16.h>
#include <math.h>

ros::NodeHandle * nh_ptr;

int g_current_wobbler_angle;
bool g_callback_received;
std::string g_motor_angle_topic_name;

void frontAngleCB(const std_msgs::Int16& angleHolder)
{
    g_current_wobbler_angle = angleHolder.data;
    g_callback_received = true;
    ROS_INFO("frontAngleCB called, received angle value of = %d", g_current_wobbler_angle);
}

// Waits for GLOBALLY DEFINED BOOLEANS to become true for a set number of seconds that are triggered by specific ros topics
// Quick and dirty ROS-based initialization
bool waitForSubs()
{
    // To get 
    ros::Subscriber front_angle_sub = nh_ptr->subscribe(g_motor_angle_topic_name, 1, frontAngleCB);
    // NIMPL: ros::Subscriber rear_angle_sub = nh_ptr->subscribe("rear_wobbler/angle", 1, rearAngleCB);

    int count = 0;
    int time_to_wait = 5;
    ros::Rate count_rate(1);

    while(count < time_to_wait)
    {
        if((g_callback_received == true))
        {
            return true;
        }
        ros::spinOnce();
        count_rate.sleep();
        count++;
    }
    return false;
}

int main(int argc, char **argv) {
    ros::init(argc, argv, "dynamixel_motor_wobble"); 
    ros::NodeHandle nh; // Can make "...("~");" if needed.
    nh_ptr = &nh;

    g_motor_angle_topic_name = "dynamixel_motor1_ang"; // CHANGE THIS TO "front_wobbler/angle" for use with final, physical system. Currently configured for an old ROSbag
    
    ros::Publisher front_motor_pub = nh.advertise<std_msgs::Int16>("front_wobbler/cmd", 1);
    ros::Publisher rear_motor_pub = nh.advertise<std_msgs::Int16>("rear_wobbler/cmd", 1);

    g_callback_received = false;
    // C++ variable for the commanded angle. This value goes "into" the ROS message object.
    short int command;

    if(!waitForSubs())
    {
        ROS_WARN("WARNING: COULD NOT FIND WOBBLER ANGLE SUBSCRIPTION! Using a default value");
        command = 1000;
    }
    else
    {
        command = g_current_wobbler_angle;
    }
    // Let user know what initial value of the angle we are going to command
    ROS_INFO("Initial wobbler motor angle (front and rear) being commanded to %d", command);

    // Controls whether the wobbler motors will increase or decrease in angle initially.
    bool increasing = false;
    int loop_counter = 0;
    // The lowest and highest angles to go to
    double min_ang;
    double max_ang;
    // How much to icnrease the angle command by each iteration. This controls the wobblers angular speed (rad/s) as a proxy variable (count/iteration)
    double change_ang;

    // ROS message object for the commanded angle
    std_msgs::Int16 int_angle; 

    // For timing the below main program 'while' loop
    double dt = 0.01;
    ros::Rate naptime(1/dt);

    // do work here in infinite loop (desired for this example)
    while (ros::ok()) 
    {
        // Only check params every X loops
        if (loop_counter % 10 == 0)
        {
            // Check if controller parameters are available. If not, choose some defaults
            if(!nh.getParam("min_ang", min_ang))
            {
                min_ang = 500;
            }

            if(!nh.getParam("max_ang", max_ang))
            {
                max_ang = 1000;  
            }

            if(!nh.getParam("wobble_speed", change_ang))
            {
                change_ang = 2;
            }
        }

        loop_counter++;

        // Cast data to a "short int" so it will be compatible with ROS
        min_ang = (short int)min_ang;
        max_ang = (short int)max_ang;
        change_ang = (short int)change_ang;

        // Change our commanded angle as appropriate
        if(increasing == true)
        {
        	command = command + change_ang;
        }
        else
        {
            command = command - change_ang;
        }

        // If we reach the maximum or minimum angle, either stop or start increasing respectively as necessary
        if(command >= max_ang)
        {
        	increasing = false;
        }
        else if(command <= min_ang)
        {
        	increasing = true;
        }

        // Load our data into ROS message
        int_angle.data = command;
        //ROS_INFO("Sending angle command of %d to both motors.", command);

        // Move both motors by the same angle, for convenienve and simplicity
        front_motor_pub.publish(int_angle); 
        rear_motor_pub.publish(int_angle);
        naptime.sleep(); 
    }
}

