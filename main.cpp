#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>
#include <thread>
#include <getopt.h> // Include getopt.h for command-line argument parsing
#include <fstream>
#include <locale>
#include <codecvt> // Include codecvt for UTF-8 conversion

using namespace std;
using namespace cv;

#define MODE_INVERTED -1
#define MODE_NORMAL 0
#define MODE_LOW_CONTRAST 1
#define MODE_BINARY 2
#define MODE_UNICODE 3


std::string getFileExtension(const std::string& filePath) {
    size_t dotPos = filePath.find_last_of('.');
    if (dotPos != std::string::npos) {
        return filePath.substr(dotPos);
    }
    return ""; // No extension found
}


wstring pixelToAscii(int pixel_intensity, int mode) {
    wstring s = L"";

    const wstring ASCII_CHARS_INVERTED = L" .:-=+*#%@";
    const wstring ASCII_CHARS_NORMAL = L"@&%#*+=-:,. ";
    const wstring ASCII_CHARS_LOW_CONTRAST = L"@#W$9876543210?!abc;:+=-,._ ";
    const wstring ASCII_CHARS_BINARY = L"@ ";
    const wstring UNICODE_CHARS = L"█▓▒░";
    switch (mode)
    {
    case -1:
        s = wstring(1, ASCII_CHARS_INVERTED[pixel_intensity * ASCII_CHARS_INVERTED.length() / 256]);
        break;
    case 1:
        s = wstring(1, ASCII_CHARS_LOW_CONTRAST[pixel_intensity * ASCII_CHARS_LOW_CONTRAST.length() / 256]);
        break;
    case 2:
        s = wstring(1, ASCII_CHARS_BINARY[pixel_intensity * ASCII_CHARS_BINARY.length() / 256]);
        break;
    case 3:
        s = wstring(1, UNICODE_CHARS[pixel_intensity * UNICODE_CHARS.length() / 256]);
        break;
    default:
        s = wstring(1, ASCII_CHARS_NORMAL[pixel_intensity * ASCII_CHARS_NORMAL.length() / 256]);
        break;
    }
    return s;
}

int main(int argc, char** argv) {
    int mode = MODE_NORMAL;
    int DISCORD_MODE = 0;

    string filePath;

    wofstream output_file;
    output_file.imbue(locale(output_file.getloc(), new codecvt_utf8<wchar_t>)); // Enable UTF-8 encoding
    string outputFilePath = "output.txt"; // Default output file name

    int width = 150;
    int height = 50;

    int option;
    while ((option = getopt(argc, argv, "m:f:s:d:h")) != -1) {
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
            case 's':
                if (strcmp(optarg, "16:9") == 0){
                    width = 16*8;
                    height = 9*5;
                }
                else if (strcmp(optarg, "1:1") == 0){
                    width = 80;
                    height = 50;
                }
                else{
                    width = 150;
                    height = 50;
                }
                break; // Add break statement here to prevent falling through to the next case

            case 'd' :
                if (strcmp(optarg, "true") == 0){
                    if(getFileExtension(filePath) == ".jpg" ||
                        getFileExtension(filePath) == ".jpeg" ||
                        getFileExtension(filePath) == ".png" ){
                            DISCORD_MODE = 1;
                            width = 7 * 8;
                            height = 5 * 8;
                            mode = MODE_UNICODE;
                    }
                    else{
                        cout << "Please give a parameter <true/false>" << endl;
                        cout << getFileExtension(filePath) << endl;
                        return -1;
                    }
                }

                break;
            case 'h':
                cout << "-m <NORMAL|INVERTED|LOW_CONTRAST|BINARY|UNICODE>     -OPTIONAL- Set a Mode for your video or image. Will be NORMAL, if nothing else is set" << endl;
                cout << "-f <file>                                            -NECESSARY- Insert file to convert into ascii" << endl;
                cout << "-h Shows you this list of commands" << endl;
                return 0;
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

    if (DISCORD_MODE){
        Mat image = imread(filePath);
        if (image.empty()) {
            cerr << "Error opening image file: " << filePath << endl;
            return -1;
        }

        cvtColor(image, image, COLOR_BGR2GRAY);
        resize(image, image, Size(width, height), 0, 0, INTER_CUBIC);

        wstring ascii_image;
        for (int i = 0; i < height; i++) {
            ascii_image += L"|";
            for (int j = 0; j < width; j++)
            {
                uchar pixel_value = image.at<uchar>(i, j);
                ascii_image += pixelToAscii(pixel_value, mode);
            }
            ascii_image += L"\n";
        }

        output_file.open("out.txt");
        output_file << ascii_image; // Write the ASCII art to the file
        output_file.close(); // Close the output file#
        wcout << ascii_image << endl;
    }
    else
    {

        VideoCapture cap(filePath);
        if (!cap.isOpened()) {
            cerr << "Error opening video file: " << filePath << endl;
            return -1;
        }

        double fps = cap.get(cv::CAP_PROP_FPS);
        double frame_duration_ms = 1000 / fps;

        Mat frame, gray_frame, resized_frame;

        while (true) {
            cap >> frame;
            if (frame.empty())
                break;

            cvtColor(frame, gray_frame, COLOR_BGR2GRAY);
            resize(gray_frame, resized_frame, Size(width, height), 0, 0, INTER_CUBIC);

            wstring ascii_frame;
            for (int i = 0; i < height; i++) {
                for (int j = 0; j < width; j++) {
                    uchar pixel_value = resized_frame.at<uchar>(i, j);
                    ascii_frame += pixelToAscii(pixel_value, mode);
                }
                ascii_frame += L"\n";
            }

            system("clear");
            wcout << ascii_frame;
            this_thread::sleep_for(chrono::milliseconds(static_cast<int>(frame_duration_ms)));
        }

    }

    return 0;
}