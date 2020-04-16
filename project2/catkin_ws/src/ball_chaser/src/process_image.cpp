#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>

// Define a global client that can request services
ros::ServiceClient client;

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
    // Request a service and pass the velocities to it to drive the robot
    ball_chaser::DriveToTarget service;
    service.request.linear_x = (float)lin_x;
    service.request.angular_z = (float)ang_z;

    // Call the safe_move service and pass the requested joint angles
    if (!client.call(service))
        ROS_ERROR("Failed to call service command_robot");
}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{
    int white_pixel = 255;

    // Loop through each pixel in the image and check if there's a bright white one
    // Then, identify if this pixel falls in the left, mid, or right side of the image
    // Depending on the white ball position, call the drive_bot function and pass velocities to it
    // Request a stop when there's no white ball seen by the camera

    int i = 0;
    int third = img.step / 3;
    for (int y = 0; y < img.height; y++) {
        int regions[3] = {0};

        for (int x = 0; x < img.step; x++) {
            int region = x / third; 
            if (img.data[i] == white_pixel) {
                regions[region]++;
            }

            i++;
        }

        // left > center
        if (regions[0] > regions[1]) {
            drive_robot(0.1, 0.1);
            break;            
        }
        // right > center
        else if (regions[2] > regions[1]) {
            drive_robot(0.1, -0.1);
            break;            
        }
        // if any white pixels in the center
        else if (regions[1] > 0) {
            drive_robot(50, 0);
            break;
        }

        if (y == img.height - 1) {
            drive_robot(0, 0);
        }
    }
}

int main(int argc, char** argv)
{
    // Initialize the process_image node and create a handle to it
    ros::init(argc, argv, "process_image");
    ros::NodeHandle n;

    // Define a client service capable of requesting services from command_robot
    client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

    // Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
    ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

    // Handle ROS communication events
    ros::spin();

    return 0;
}