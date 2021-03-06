/******************************
 *  [Header]
 *  Point Transporter and Navigator
 *  Receive Custom Goal Message from Upper layer
 *  and traslate to "goalMessage"
 *  Author : Theppasith Nisitsukcharoen
 *  29-Sept-2016
 *******************************/

#include <ros/ros.h>
#include <ros/package.h>
#include <std_srvs/Empty.h>
#include <std_msgs/Empty.h>
#include <nav_msgs/Odometry.h>
#include <geometry_msgs/Pose2D.h>
#include <geometry_msgs/PoseStamped.h>
#include <tf/transform_listener.h>
#include <actionlib/client/simple_action_client.h>
#include <actionlib_msgs/GoalID.h>
#include <move_base_msgs/MoveBaseAction.h>

// MoveBase Actionlib typedef
typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> MoveBaseClient;

class PointTransporter
{
    public:
    PointTransporter();
    ~PointTransporter();

    private:
    ros::NodeHandle   nh_;                   // Nodehandle
    ros::Subscriber   upperLevelGoalSub;     // Subscriber to UpperLevel
    ros::Subscriber   stopGoalSub;           // StopGoal Subscriber
    ros::Publisher    goalMarkerPub;         // RViz Goal Marker
    ros::Publisher    moveBaseStopPub;       // move_base canceGoal Pub
    std::string       upperLevelTopic;       // Upper level topic name
    std::string       actionTopicNamespace;  // move_base goal topic
    std::string       stopGoalTopic;         // stopGoal Topic (move_base/cancel wrapper)
    MoveBaseClient    *actionClient;         // Client to contact with move_base actionlib
    move_base_msgs::MoveBaseGoal  marked_current_pos;
    move_base_msgs::MoveBaseGoal  marked_goal;

    // On Received Goal from Upper Level
    void onReceiveGoalCallback(
      const move_base_msgs::MoveBaseGoalConstPtr  &receivedGoal);

    // Feedback while traversing
    void goalFeedbackCallback(
      const move_base_msgs::MoveBaseFeedbackConstPtr &feedback);

    // Feedback while goal is active
    void goalDoneCallback(
      const actionlib::SimpleClientGoalState &state,
      const move_base_msgs::MoveBaseResultConstPtr &result);

    // Feedback on Goal is finished.
    void goalActiveCallback();

    // Action when receive stop message
    void stopGoalCallback(
      const std_msgs::EmptyConstPtr &msg
    );

};


// Class Constructor
PointTransporter::PointTransporter(void){
    // Default values
    upperLevelTopic          = "/kamtoa/goal";
    actionTopicNamespace     = "move_base";
    stopGoalTopic            = "/kamtoa/cancel";

    // Get parameters from parameter server
    nh_.param("upper_level_topic"     ,upperLevelTopic,  upperLevelTopic   );
    nh_.param("action_topic_namespace",actionTopicNamespace  ,  actionTopicNamespace   );
    nh_.param("stop_goal_topic"     ,stopGoalTopic,  stopGoalTopic   );


    // Initiate the actionlib client
    actionClient       = new MoveBaseClient(actionTopicNamespace , true);

    // Initiate Publisher and Subscriber
    upperLevelGoalSub  = nh_.subscribe<move_base_msgs::MoveBaseGoal>(upperLevelTopic, 10,
                                         &PointTransporter::onReceiveGoalCallback, this);
    // Marker Visualizer (RVIZ) Publisher
    goalMarkerPub      = nh_.advertise<geometry_msgs::PoseStamped>("/move_base_simple/goal" , 10);

    // Move base Cancel Goal ActionLib Publisher
    moveBaseStopPub    = nh_.advertise<actionlib_msgs::GoalID>("/move_base/cancel",1);

    // Subscriber for Goal Cancelling command
    stopGoalSub        = nh_.subscribe<std_msgs::Empty>(stopGoalTopic,1,
                                        &PointTransporter::stopGoalCallback, this);

}


PointTransporter::~PointTransporter(void){
    std::cout << std::endl;
    ROS_INFO("Destructor Called : Remove every instance");

    // [TODO:]Stop Every move_base command

    delete actionClient;
}



void PointTransporter::stopGoalCallback(
    const std_msgs::EmptyConstPtr &msg)
{
    ROS_INFO("STOP GOAL received -> PUBLISH move_base/cancel");
    // Do things when we press STOP BUTTON!
    // TODO: Set the CMD_VEL_MUX to FOCUS TO 0,0,0 m/s
    // until the cancel goal returns good state
    moveBaseStopPub.publish(*new actionlib_msgs::GoalID());

}

void PointTransporter::onReceiveGoalCallback(
    const move_base_msgs::MoveBaseGoalConstPtr  &receivedGoal)
{
    // On received goal from upper layer
    ROS_INFO("Got the new Goal (x,y,yaw) from Upper Layer");

    // [TODO:] Set the speed to zero and set the heading to the free area
    // Adjust Heading

    // Start the navigation process
    move_base_msgs::MoveBaseGoal goal;

    // Set the goal message's payload
    goal.target_pose.header.frame_id = "/map";
    goal.target_pose.header.stamp = ros::Time::now();

    goal.target_pose.pose.position.x    = receivedGoal->target_pose.pose.position.x;
    goal.target_pose.pose.position.y    = receivedGoal->target_pose.pose.position.y;
    goal.target_pose.pose.orientation.x = receivedGoal->target_pose.pose.orientation.x;
    goal.target_pose.pose.orientation.y = receivedGoal->target_pose.pose.orientation.y;
    goal.target_pose.pose.orientation.z = receivedGoal->target_pose.pose.orientation.z;
    goal.target_pose.pose.orientation.w = receivedGoal->target_pose.pose.orientation.w;

    // Send Goal to Navigation Stack
    actionClient->sendGoal(goal,
                boost::bind(&PointTransporter::goalDoneCallback, this , _1, _2),
                boost::bind(&PointTransporter::goalActiveCallback , this),
                boost::bind(&PointTransporter::goalFeedbackCallback,  this ,_1)
              );

    // publish the visualization marker /move_base_simple/goal
    goalMarkerPub.publish(goal.target_pose);

}

void PointTransporter::goalFeedbackCallback(
    const move_base_msgs::MoveBaseFeedbackConstPtr &feedback)
{
    // Goal is active and send feedback
}

void PointTransporter::goalDoneCallback(
    const actionlib::SimpleClientGoalState &state,
    const move_base_msgs::MoveBaseResultConstPtr &result)
{
    // Goal is done (error , success)

}

void PointTransporter::goalActiveCallback(){
    // Goal is Activing
}
