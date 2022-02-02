#pragma once
#include <vector>
#include "opencv2/opencv.hpp"

class VideoData;
class QFile;

constexpr double EPS = .0000001;

enum class datasettype {
    ONESTACK,
    MULTIPLESTACKS,
    SINGLEFRAMES,
    DEADPIX
};
class filescopeguard {
public:
    filescopeguard(QFile* f);
    ~filescopeguard();
private:
    QFile* file;
};


//void generatedatasets();
void loaddataset(VideoData* data, datasettype = datasettype::ONESTACK, double framerate = 10., int downspace = 1, int downtime = 1);
template<class Ta, class Tb> inline bool nearlyequal(Ta a, Tb b){ return abs(a - b) < EPS; }
template<class T> std::vector<std::vector<T>> getMat2d(cv::Mat* mat) {
    auto sz = mat->size();
    std::vector<std::vector<T>> ret = std::vector<std::vector<T>>(sz.height);
    
    for (int i = 0; i < sz.height; ++i) {
        ret[i].resize(sz.width);
        for (int j = 0; j < sz.width; ++j) {
            ret[i][j] = mat->at<T>(i, j);
        }
    }
    return ret;
}