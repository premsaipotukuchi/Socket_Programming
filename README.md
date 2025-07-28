README - ICSI 500 Project 2
Operating Systems - Project 2: Multithreaded Client-Server-Helper Vowel Processing System
University at Albany - Department of Computer Science
Student: Venkata Manikanta Prem Sai Potukuchi
Submission Due Date: December 5th, 2024

-------------------------------------------------------------------------------
PROJECT OVERVIEW
-------------------------------------------------------------------------------
This project implements a multithreaded client-server-helper system using C, 
POSIX threads, semaphores, and Linux TCP sockets. The system is designed to 
analyze documents and report the total count of vowels (a, e, i, o, u). 
All communication between the components is performed using encoded frames 
transmitted via sockets.

-------------------------------------------------------------------------------
DIRECTORY STRUCTURE
-------------------------------------------------------------------------------
/project2/
│
├── client.c                 # Source code for the Client
├── server.c                 # Source code for the Server
├── helper.c                 # Source code for the Helper
├── queue.c / queue.h        # Thread-safe message queue implementation
├── shared.h                 # Shared constants, socket configs, and data types
├── Makefile                 # Makefile to build all components
├── input.txt                # Input file with raw text to be processed
├── vowelCount.txt           # Output file from the server with vowel counts
├── receivedVowelCount.txt   # Output file from the client after decoding
├── documentation.docx       # External documentation (system design, tests)
├── sample_test_cases/       # Folder containing test input/output samples
└── README.txt               # This readme file

-------------------------------------------------------------------------------
HOW TO COMPILE
-------------------------------------------------------------------------------
Make sure you have gcc and pthread libraries installed.
Run the following command in the project directory:

    make all

This will create three executables:
- `client`
- `server`
- `helper`

To clean up compiled files:

    make clean

-------------------------------------------------------------------------------
HOW TO RUN
-------------------------------------------------------------------------------
**Step 1:** Launch the Helper
    ./helper

**Step 2:** Launch the Server (in a separate terminal)
    ./server

**Step 3:** Launch the Client (in another terminal)
    ./client

Ensure that all sockets are using the same ports and IPs as specified in `shared.h`.

-------------------------------------------------------------------------------
SOURCE FILE DESCRIPTIONS
-------------------------------------------------------------------------------
- `client.c`: 
    - Reads from `input.txt`
    - Sends encoded data to helper
    - Receives decoded frame from server and decodes it via helper
    - Saves result in `receivedVowelCount.txt`

- `server.c`: 
    - Receives data from client
    - Uses multithreaded pipeline (charA to charU + count thread)
    - Sends result to helper for encoding
    - Sends encoded data back to client

- `helper.c`: 
    - Performs encoding and decoding using binary + parity
    - Shared with both client and server

- `queue.c / queue.h`: 
    - Implements thread-safe queues with semaphores
    - Used for inter-thread communication in the server

- `shared.h`: 
    - Contains port numbers, socket IPs, constants, and shared struct definitions

-------------------------------------------------------------------------------
NOTES
-------------------------------------------------------------------------------
- Each component must be run in a separate terminal.
- Ensure `input.txt` is available before running the client.
- Output files `vowelCount.txt` and `receivedVowelCount.txt` are overwritten every run.
- No error checking or retransmission logic is implemented as per the assignment specification.

-------------------------------------------------------------------------------
CONTACT
-------------------------------------------------------------------------------
For any questions regarding this project, please reach out via email:

-------------------------------------------------------------------------------
