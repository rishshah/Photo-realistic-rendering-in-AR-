#include <iostream>
#include <algorithm>
#include <fstream>
#include <chrono>
#include <opencv2/core/core.hpp>
#include <System.h>

using namespace std;


const vector<string> explode(const string& s, const char& c) {
    string buff{""};
    vector<string> v;

    for (auto n : s)
    {
        if (n != c) buff += n; else if (n == c && buff != "") { v.push_back(buff); buff = ""; }
    }
    if (buff != "") v.push_back(buff);

    return v;
}


double f(int i){
    return i / 1000.0;
}
void LoadImages(const string &strImagePath, const string &strPathTimes,
                vector<string> &vstrImages, vector<double> &vTimeStamps) {
    ifstream fTimes;
    fTimes.open(strPathTimes.c_str());
    vTimeStamps.reserve(5000);
    vstrImages.reserve(5000);
    while (!fTimes.eof())
    {
        string s;
        getline(fTimes, s);
        if (!s.empty())
        {
            stringstream ss;
            ss << s;
            std::vector<string> v{explode(ss.str(), ',')};
            vstrImages.push_back(strImagePath + "/" + v[1]);
            double t = atof(v[0].c_str());
            vTimeStamps.push_back(t / 1e9);

        }
    }
}

int image_mode(char **argv) {
    // Retrieve paths to images
    vector<string> vstrImageFilenames;
    vector<double> vTimestamps;
    LoadImages(string(argv[3]), string(argv[4]), vstrImageFilenames, vTimestamps);

    int nImages = vstrImageFilenames.size();

    if (nImages <= 0) {
        cerr << "ERROR: Failed to load images" << endl;
        return 1;
    }

    // Create SLAM system. It initializes all system threads and gets ready to process frames.
    ORB_SLAM2::System SLAM(argv[1], argv[2], ORB_SLAM2::System::MONOCULAR, true);

    // Vector for tracking time statistics
    vector<float> vTimesTrack;
    vTimesTrack.resize(nImages);

    cout << endl << "-------" << endl;
    cout << "Start processing sequence ..." << endl;
    cout << "Images in the sequence: " << nImages << endl << endl;

    // Main loop
    cv::Mat im;
    for (int ni = 0; ni < nImages; ni++) {
        // Read image from file
        im = cv::imread(vstrImageFilenames[ni], CV_LOAD_IMAGE_UNCHANGED);
        double tframe = vTimestamps[ni];

        if (im.empty()) {
            cerr << endl << "Failed to load image at: " <<  vstrImageFilenames[ni] << endl;
            return 1;
        }

        std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();

        // Pass the image to the SLAM system
        SLAM.TrackMonocular(im, tframe);
        std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();

        double ttrack = std::chrono::duration_cast<std::chrono::duration<double> >(t2 - t1).count();
        vTimesTrack[ni] = ttrack;

        // Wait to load the next frame
        double T = 0;
        if (ni < nImages - 1)
            T = vTimestamps[ni + 1] - tframe;
        else if (ni > 0)
            T = tframe - vTimestamps[ni - 1];

        if (ttrack < T)
            usleep((T - ttrack) * 1e6);
    }

    // Stop all threads
    SLAM.Shutdown();

    // Tracking time statistics
    sort(vTimesTrack.begin(), vTimesTrack.end());
    float totaltime = 0;
    for (int ni = 0; ni < nImages; ni++)
    {
        totaltime += vTimesTrack[ni];
    }
    cout << "-------" << endl << endl;
    cout << "median tracking time: " << vTimesTrack[nImages / 2] << endl;
    cout << "mean tracking time: " << totaltime / nImages << endl;

    // Save camera trajectory
    SLAM.SaveKeyFrameTrajectoryTUM("KeyFrameTrajectory.txt");
    return 0;
}

int webcam_mode(char **argv) {

    int nImages = 1000;

    // Create SLAM system. It initializes all system threads and gets ready to process frames.
    ORB_SLAM2::System SLAM(argv[1], argv[2], ORB_SLAM2::System::MONOCULAR, true);

    // Vector for tracking time statistics
    vector<float> vTimesTrack;
    vTimesTrack.resize(nImages);

    cv::VideoCapture cap;
    if(!cap.open(0))
        return 1;
    cv::Mat im;
    for (int ni = 0; ni < nImages; ni++) {
        // Read image from file
        cap >> im;
        double tframe = f(ni);

        std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();

        // Pass the image to the SLAM system
        SLAM.TrackMonocular(im, tframe);
        std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();

        double ttrack = std::chrono::duration_cast<std::chrono::duration<double> >(t2 - t1).count();
        vTimesTrack[ni] = ttrack;

        // Wait to load the next frame
        double T = 0;
        if (ni < nImages - 1)
            T = f(ni + 1) - tframe;
        else if (ni > 0)
            T = tframe - f(ni - 1);

        if (ttrack < T)
            usleep((T - ttrack) * 1e6);
    }

    // Stop all threads
    SLAM.Shutdown();

    // Tracking time statistics
    sort(vTimesTrack.begin(), vTimesTrack.end());
    float totaltime = 0;
    for (int ni = 0; ni < nImages; ni++)
    {
        totaltime += vTimesTrack[ni];
    }
    cout << "-------" << endl << endl;
    cout << "median tracking time: " << vTimesTrack[nImages / 2] << endl;
    cout << "mean tracking time: " << totaltime / nImages << endl;

    // Save camera trajectory
    SLAM.SaveKeyFrameTrajectoryTUM("KeyFrameTrajectory.txt");
    return 0;
}

int main(int argc, char **argv) {
    if (argc != 5 and argc != 3) {
        cerr << argv[0] << endl;
        cerr << argv[1] << endl;
        cerr << endl << "Usage: ./exec path_to_vocabulary path_to_settings or \nUsage: ./exec path_to_vocabulary path_to_settings path_to_image_folder path_to_times_file" << endl;
        return 1;
    }

    int ret = -1;
    if (argc == 5) {
        ret = image_mode(argv);
    } else {
        ret = webcam_mode(argv);
    }
    if (ret != 0) {
        cout << "ERROR" << endl;
    }
    return ret;
}