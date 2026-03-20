# Main Application CMakeLists.txt

idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES motor_control web_control servo_control wall_follower sensor_manager
)
