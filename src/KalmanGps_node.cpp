#include "ros/ros.h"

#include <sensor_msgs/NavSatFix.h>
#include <sensor_msgs/FluidPressure.h>

#include "std_msgs/String.h"

#include "KalmanFilter.h"

#include <Eigen/Core>

#define AIR_MOLAR_MASS 	0.02897
#define GAZ_CSTE 		8.3144598

#define DEF_TEMP 		298.15

#define SIGMA_Z_GPS		5.0 	//[m]
#define SIGMA_Z_BARO	5.0		//[pa]

KalmanFilter* filter = 0;

double pressure = 0;

void gps_update(const sensor_msgs::NavSatFix message) {
	if(filter) {
		filter->update_gps(message.altitude, SIGMA_Z_GPS);
	}
	else if(pressure != 0) {
		// Init the filter
		Eigen::Matrix<double, 6, 1, Eigen::DontAlign> x0;
		x0(0) = message.altitude;
		x0(1) = 0.0;
		x0(2) = 0.0;
		x0(3) = message.altitude;
		x0(4) = AIR_MOLAR_MASS / (GAZ_CSTE * DEF_TEMP);
		x0(5) = pressure;

		Eigen::Matrix<double, 6, 1, Eigen::DontAlign> p0 = Eigen::Matrix<double, 6, 1, Eigen::DontAlign>::Zero();
		p0(0, 0) = pow(5.0, 2);
		p0(1, 1) = pow(0.5, 2);
		p0(2, 2) = pow(0.5, 2);
		p0(3, 3) = pow(5.0, 2);
		p0(4, 4) = pow(1e-6, 2);
		p0(5, 5) = pow(5.0, 2);
		
		//filter = new KalmanFilter(x0, p0);

		ROS_INFO("Filter init !");
	}
}

void baro_update(const sensor_msgs::FluidPressure message) {
	if(filter) {
		filter->update_baro(message.fluid_pressure, SIGMA_Z_BARO);
	}
	else {
		pressure = message.fluid_pressure;
	}
}

int main(int argc, char **argv)
{
	ros::NodeHandle n;

	ros::init(argc, argv, "KalmanGPS");

	ros::Subscriber _subNavSatFix = n.subscribe<sensor_msgs::NavSatFix>("/iris1/global_position/raw/fix", 1, gps_update);
	ros::Subscriber _subFluidPressure = n.subscribe<sensor_msgs::FluidPressure>("/iris1/imu/atm_pressure",1, baro_update);

	ros::spin();

	return 0;
}