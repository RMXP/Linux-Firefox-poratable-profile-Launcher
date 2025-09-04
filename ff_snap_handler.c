#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <sys/wait.h>

#define MAX_PASSWORD_LENGTH 100
#define FLASH_DRIVE_PROFILE_PATH "/PortableApps/FirefoxPortable/Data/profile/" // Adjusted path

#define BUFFER_SIZE 2048 // Define a constant for the buffer size



#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SIZE 2048 // Define a buffer size for reading output




#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SIZE 2048 // Define a buffer size for reading output

int execute_command(const char *command) {
    FILE *fp;
    char buffer[BUFFER_SIZE];
    int exit_status;

    // Open the command for reading
    fp = popen(command, "r");
    if (fp == NULL) {
        perror("popen failed");
        return -1; // Return -1 to indicate an error
    }

    // Read the output a line at a time - output it.
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        printf("%s", buffer); // Print the output to standard output
    }
    printf("\n");
    
    // Close the command stream and get the exit status
    exit_status = pclose(fp);
    if (exit_status == -1) {
        perror("pclose failed");
        return -1; // Return -1 to indicate an error
    }

    // Return the exit status of the command
    return WEXITSTATUS(exit_status);
}


int is_snap_connected() {
    FILE *fp;
    char path[BUFFER_SIZE]; // Use the defined buffer size
    int connected = 0;

    // Run the command to check the state of snap firefox removable-media
    fp = popen("snap info firefox | grep 'removable-media'", "r");
    if (fp == NULL) {
        perror("Failed to run command"); // Use perror for better error reporting
        return -1; // Indicate error
    }

    // Read the output
    while (fgets(path, sizeof(path), fp) != NULL) {
        if (strstr(path, "connected") != NULL) {
            connected = 1; // Snap is connected
            break;
        }
    }

    pclose(fp); // Ensure the file pointer is closed
    return connected; // Return 1 for connected, 0 for not connected
}


int connect_snap(const char *password) {
    // Create a command to connect snap using sudo and the provided password
    char command[256];
    snprintf(command, sizeof(command), "echo '%s' | sudo -S snap connect firefox:removable-media", password);
    
    int status = execute_command(command);
    if (status == -1) {
        perror("system call failed");
        return -1; // Return -1 for system call failure
    } else {
        return WEXITSTATUS(status); // Return the exit code of the command
    }
}

int disconnect_snap(const char *password) {
    // Create a command to disconnect snap using sudo and the provided password
    char command[256];
    snprintf(command, sizeof(command), "echo '%s' | sudo -S snap disconnect firefox:removable-media", password);
    
    int status = execute_command(command);
    if (status == -1) {
        perror("system call failed");
        return -1; // Return -1 for system call failure
    } else {
        return WEXITSTATUS(status); // Return the exit code of the command
    }
}


// Function to read password without echo
int read_password(char *password, size_t max_length) {
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt); // Get the current terminal settings
    newt = oldt; // Copy the settings
    newt.c_lflag &= ~(ECHO); // Disable echo
    tcsetattr(STDIN_FILENO, TCSANOW, &newt); // Set the new settings

    // Get the username from the environment variable
    const char *username = getenv("USER");
    if (username == NULL) {
        username = "user"; // Fallback if USER is not set
    }

    printf("[ff_snap_handler] password for %s: ", username); // Prompt following the specified format
    if (fgets(password, max_length, stdin) == NULL) {
        perror("Error reading password");
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt); // Restore the old settings
        return -1; // Indicate failure
    }
    password[strcspn(password, "\n")] = 0; // Remove newline character

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt); // Restore the old settings
    printf("\n"); // Print a newline after password input
    return 0; // Indicate success
}


int main(int argc, char *argv[]) {
    char password[MAX_PASSWORD_LENGTH];
    int disconnect_after = 0;

    // Get the current working directory
    char cwd[256];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd() error");
        return 1; // Exit with an error code
    }

    // Get the maximum command length (ARG_MAX)
    long max_command_length = sysconf(_SC_ARG_MAX);
    if (max_command_length == -1) {
        perror("sysconf error");
        return 1; // Exit with an error code
    }

    // Construct the command to run Firefox
    char *command = malloc(max_command_length);
    if (command == NULL) {
        perror("malloc error");
        return 1; // Exit with an error code
    }

    snprintf(command, max_command_length, "snap run firefox -profile \"%s%s\" ", cwd, FLASH_DRIVE_PROFILE_PATH);

    // Append any additional arguments passed to the program
    for (int i = 1; i < argc; i++) {
        strncat(command, argv[i], max_command_length - strlen(command) - 1);
        if (i < argc - 1) {
            strncat(command, " ", max_command_length - strlen(command) - 1); // Add space between arguments
        }
    }

    // Check if snap firefox removable-media is connected
    int con_state = is_snap_connected();
    if (con_state) {
        // snap was connected to removable-media
        printf("removable-media: connected\n");
    }
    else {
        // snap was disconnected from removable-media
        printf("removable-media: disconnected\n");
        printf("Connecting... \n");
    
        if (read_password(password, MAX_PASSWORD_LENGTH) != 0) {
            printf("Failed to read password. Exiting.\n");
            free(command);
            return 1; // Exit with an error code
        }
        int result = connect_snap(password);
        if (result != 0) {
            printf("Connection failed. Exit code: %d\n", result);
            free(command);
            return result; // Exit the program with the error code
        }
        else {
            printf("Connection success!\n");
        }
        disconnect_after = 1; // Set disconnect_after flag
    }

    // Run the constructed command
    int status = execute_command(command);
    if (status == -1) {
        perror("system call failed");
        free(command);
        return 1; // Exit with an error code
    }

    // If disconnect_after is set, disconnect snap firefox removable-media
    if (disconnect_after) {
        printf("Disconnecting removable-media... ");
        int result = disconnect_snap(password);
        if (result != 0) {
            printf("Failed. Exit code: %d\n", result);
            free(command);
            return result; // Exit the program with the error code
        }
        else {
            printf("Success!\n");
        }
    }

    memset(password, 0, sizeof(password)); // Clear password from memory
    free(command); // Free the allocated memory
    return 0;
}

