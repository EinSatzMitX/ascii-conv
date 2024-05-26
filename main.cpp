#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>
#include <thread>
#include <getopt.h> // Include getopt.h for command-line argument parsing

using namespace std;
using namespace cv;

#define MODE_INVERTED -1
#define MODE_NORMAL 0
#define MODE_LOW_CONTRAST 1
#define MODE_BINARY 2
#define MODE_UNICODE 3

string pixelToAscii(int pixel_intensity, int mode) {
    string s = "";

    const string ASCII_CHARS_INVERTED = " .:-=+*#%@";
    const string ASCII_CHARS_NORMAL = "@&%#*+=-:,. ";
    const string ASCII_CHARS_LOW_CONTRAST = "@#W$9876543210?!abc;:+=-,._ ";
    const string ASCII_CHARS_BINARY = "@ ";
    const string UNICODE_CHARS = "█▓▒░ ";
    switch (mode)
    {
    case -1:
        s = string(1, ASCII_CHARS_INVERTED[pixel_intensity * ASCII_CHARS_INVERTED.length() / 256]);
        break;
    case 1:
        s = string(1, ASCII_CHARS_LOW_CONTRAST[pixel_intensity * ASCII_CHARS_LOW_CONTRAST.length() / 256]);
        break;
    case 2:
        s = string(1, ASCII_CHARS_BINARY[pixel_intensity * ASCII_CHARS_BINARY.length() / 256]);
        break;
    case 3:
        s = string(1, UNICODE_CHARS[pixel_intensity * UNICODE_CHARS.length() / 256]);
        break;
    default:
        s = string(1, ASCII_CHARS_NORMAL[pixel_intensity * ASCII_CHARS_NORMAL.length() / 256]);
        break;
    }
    return s;
}

int main(int argc, char** argv) {
    int mode = MODE_NORMAL; // Default mode
    string filePath;

    // Parse command-line arguments
    int option;
    while ((option = getopt(argc, argv, "m:f:")) != -1) {
        switch (option) {
            case 'm':
                if (strcmp(optarg, "NORMAL") == 0) {
                    mode = MODE_NORMAL;
                } else if (strcmp(optarg, "INVERTED") == 0) {
                    mode = MODE_INVERTED;
                } else if (strcmp(optarg, "LOW_CONTRAST") == 0) {
                    mode = MODE_LOW_CONTRAST;
                } else if (strcmp(optarg, "BINARY") == 0) {
                    mode = MODE_BINARY;
                }
                else if (strcmp(optarg, "UNICODE") == 0)
                {
                    mode = MODE_UNICODE;
                }
                else
                {
                    cerr << "Invalid mode specified" << endl;
                    return -1;
                }
                break;
            case 'f':
                filePath = optarg;
                break;
            case 'h' :
                cout << "-m <NORMAL|INVERTED|LOW_CONTRAST|BINARY|UNICODE>     -OPTIONAL- Set a Mode for your video or image. Will be NORMAL, if nothing else is set" << endl;
                cout << "-f <file>                                            -NECESSARY- Insert file to convert into ascii" << endl;
                cout << "-h Shows you this list of commands" << endl;
            default:
                cerr <<"-m <NORMAL|INVERTED|LOW_CONTRAST|BINARY|UNICODE>     -OPTIONAL- Set a Mode for your video or image. Will be NORMAL, if nothing else is set" << endl;
                cerr << "-f <file>                                            -NECESSARY- Insert file to convert into ascii" << endl;
                cerr << "-h Shows you this list of commands" << endl;
                return -1;
        }
    }

    // Check if the file path is provided
    if (filePath.empty()) {
        cerr << "File path not provided" << endl;
        cerr <<"-m <NORMAL|INVERTED|LOW_CONTRAST|BINARY|UNICODE>     -OPTIONAL- Set a Mode for your video or image. Will be NORMAL, if nothing else is set" << endl;
        cerr << "-f <file>                                            -NECESSARY- Insert file to convert into ascii" << endl;
        cerr << "-h Shows you this list of commands" << endl;
        return -1;
    }

    cout << "File path: " << filePath << endl;

    VideoCapture cap(filePath);
    if (!cap.isOpened()) {
        cerr << "Error opening video file: " << filePath << endl;
        return -1;
    }

    double fps = cap.get(cv::CAP_PROP_FPS);
    double frame_duration_ms = 1000 / fps;

    int width = 150;
    int height = 50;

    Mat frame, gray_frame, resized_frame;

    while (true) {
        cap >> frame;
        if (frame.empty())
            break;

        cvtColor(frame, gray_frame, COLOR_BGR2GRAY);
        resize(gray_frame, resized_frame, Size(width, height), 0, 0, INTER_CUBIC);

        string ascii_frame;
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                uchar pixel_value = resized_frame.at<uchar>(i, j);
                ascii_frame += pixelToAscii(pixel_value, mode);
            }
            ascii_frame += "\n";
        }

        system("clear");
        cout << ascii_frame;
        this_thread::sleep_for(chrono::milliseconds(static_cast<int>(frame_duration_ms)));
    }

    return 0;
}