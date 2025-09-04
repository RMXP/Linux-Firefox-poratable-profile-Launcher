# Linux Firefox Portable Profile Launcher

## Description
This purpose of this project is to provide a user-friendly way to run Firefox with a portable profile stored on a flash drive. It automatically detects the installation method of Firefox (standard, Flatpak, or Snap) and manages the details. It ensures that the Firefox version is compatible with the profile and can attempt to update Firefox if needed.

## Justification
When I ran Windows I used FirefoxPortable to keep a customized install of Firefox. I wanted something similar when switching to Linux. Here's some of the benefits of separate profiles:

- A portable profile can be loaded on a variety of hosts and is easy to backup.
- My portable profile has private information I don't want to leave on any host computer like save logins and autofill information.
- My portable profile is set up to be maximally compatable with webistes for paying bills.
- The main profile on each of my computers have ad-blocking and are set up for more security during general browsing.

I switched to Linux because I didn't like the direction Microsoft was going  with Windows 10's telemetry and Firefox ending support for Windows 7. The Steam Deck's Linux based OS and compatibility with Windows games removed a major barrier to this switch. Attempts to run the PortableApps version of Firefox through Bottles (a Wine wrapper) were unreliable. Issues were maily due to the Wine transaltion layer, file access permissions, and the way Bottles maps Windows drives to Linux paths.

I discovered that Firefox has a command-line parameter for loading an external profile I can use to have separate, external, profiles. I had copied my external profile to a RAMDRIVE for expirimenting. The RAMDRIVE lets me read and write as much as I like without thrashing the boot drive and it can be given more permissive permissions. Since nothing gets saved permanentely it's a safe drive to play with. Well, Linux doesn't treat the RAMDRIVE the same as an external drive (USB or sdcard). It turns out each installation of Firefox requires a different method for accessing files on an external drive.

Different distros handle removable drives differently. This might also have to do with the chosen file system. My profile drive is FAT32. It seems that all files with a .exe extension are automatically given execute permission. This affected the file names in this project.

## Files
- **Linux Firefox (portable profile).sh.exe**: The main script that users will run to launch Firefox with the portable profile.
- **ff_snap_handler.c**: C source code for `ff_snap_handler.exe`, which handles the details of executing Firefox installed via Snap.
- **Readme.md**: This file you're currently reading.
- **LICENSE.txt**: Text of the MIT license. Okay to use for any reason, attribution appreciated but not required.

## Installation Instructions
1. compile the C code:
   ```bash
   gcc -o ff_snap_handler.exe ff_snap_handler.c
   ```
2. Copy "Linux Firefox (portable profile).sh.exe" and ff_snap_handler.exe to the root of your profile drive.
3. Get the path to your profile relative to the root of the drive.
4. Edit "Linux Firefox (portable profile).sh.exe" and set profile to your path.

## Usage
To run the script, execute the following command in your terminal:
```bash
./"Linux Firefox (portable profile).sh.exe"
```
The script will check versions of the installed Firefox and the external profile, updating Firefox if needed. It will only ask for the password before updating or setting the removable-media permission. You won't have to re-enter the password when exiting Firefox even for Snap.

## Dependencies
- **Firefox**: Firefox should be installed on your system via standard methods, Flatpak, or Snap. The script will tell you if it can't find an installation.
- **GCC**: Required to compile "ff_snap_handler.c".

## C Program Functionality
The `ff_snap_handler.c` program performs the following tasks:

- **Check Snap Connection**: It notes the state of Snap Firefox's removable-media interface (connected or disconnected).
- **Prompt for password**: It prompts for the password. This will be used as needed. You'll only be prompted once.
- **Connect if needed**: It ensures the removable-media interface is connected using the provided password.
- **Execute Firefox**: Run Firefox, passing along any command-line parameters. Wait for Firefox to close.
- **Reset permissions**: Resets the state of the removable-media interface to what it was when ff_snap_handler.exe was executed.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE.txt) file for details.
