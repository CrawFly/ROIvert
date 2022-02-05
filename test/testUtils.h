#pragma once
#include <vector>
#include "opencv2/opencv.hpp"
#include <QString>
#include <QFile>
class VideoData;

constexpr double EPS = .0000001;

class filescopeguard {
public:
    filescopeguard(QFile* f) : file(f) {}
    ~filescopeguard() { file->remove(); }
private:
    QFile* file;
};
enum class datasettype {
    ONESTACK,
    MULTIPLESTACKS,
    SINGLEFRAMES,
    DEADPIX
};

template<class T> class ROIVertMat3D {
    // this is a helper class for test
public:
    ROIVertMat3D() = default;
    ROIVertMat3D(std::vector<std::vector<std::vector<T>>> vecdata) {
        nframes = vecdata.size();
        if (!vecdata.empty()) {
            nrows = vecdata[0].size();
            if (!vecdata[0].empty()) {
                ncols = vecdata[0][0].size();
            }
        }

        data.reserve(nframes * nrows * ncols);
        for (const auto& frame : vecdata) {
            for (const auto& row : frame) {
                for (const auto& val : row) {
                    data.push_back(val);
                }
            }
        }
    }
    ROIVertMat3D(const std::vector<cv::Mat>& matdata) {
        nframes = matdata.size();

        if (!matdata.empty()) {
            auto sz = matdata[0].size();
            nrows = sz.height;
            ncols = sz.width;
            data.reserve(nframes * nrows * ncols);
            for (auto& frame : matdata) {
                for (int i = 0; i < nrows; ++i) {
                    for (int j = 0; j < ncols; ++j) {
                        data.push_back(frame.at<T>(i, j));
                    }
                }
            }
        }
    }
    ROIVertMat3D(std::vector<T> d, size_t nf, size_t nr, size_t nc) {
        Q_ASSERT(d.size() == nf * nr * nc);
        data = d;
        nframes = nf;
        nrows = nr;
        ncols = nc;
    }

    std::vector<std::vector<std::vector<T>>> getAsVectors() {
        std::vector<std::vector<std::vector<T>>> ret(nframes);
        auto cntr = 0;
        for (auto& frame : ret) {
            frame.resize(nrows);
            for (auto& row : frame) {
                row.resize(ncols);
                for (auto& val : row) {
                    val = data[cntr++];
                }
            }
        }
        return ret;
    }
    
    ROIVertMat3D<T> getSlice(size_t slice) {
        Q_ASSERT(slice < nframes);
        ROIVertMat3D<T> ret;
        std::vector<T> slicedata;
        slicedata.reserve(nrows * ncols);
        for (size_t i = nrows * ncols* slice; i < nrows * ncols * (slice + 1); ++i) {
            slicedata.push_back(data[i]);
        }
        return ROIVertMat3D<T>(slicedata, 1, nrows, ncols);
    }
    std::vector<T> getPixel(size_t r, size_t c) {
        Q_ASSERT(r < nrows && c < ncols);
        std::vector<T> ret(nframes);
        for (size_t f = 0; f < nframes; ++f) {
            ret[f] = data[f*nrows*ncols + r*ncols + c];
        }
        
        return ret;
    }


    static ROIVertMat3D<T> max(const ROIVertMat3D<T> &A, const ROIVertMat3D<T> &B) {
        Q_ASSERT(A.data.size() == B.data.size() &&
            A.nrows == B.nrows &&
            A.ncols == B.ncols);

        std::vector<T> maxdata(A.data.size());
        
        auto it1 = A.data.begin(), it2 = B.data.begin();
        auto it3 = maxdata.begin();
        for (; it1 < A.data.end(); ++it1, ++it2, ++it3) {
            *it3 = std::max(*it1, *it2);
        }

        return ROIVertMat3D<T>(maxdata, A.nframes, A.nrows, A.ncols);
    }
    static ROIVertMat3D<T> min(const ROIVertMat3D<T> &A, const ROIVertMat3D<T> &B) {
        Q_ASSERT(A.data.size() == B.data.size() &&
            A.nrows == B.nrows &&
            A.ncols == B.ncols);

        std::vector<T> mindata(A.data.size());
        auto it1 = A.data.begin(), it2 = B.data.begin();
        auto it3 = mindata.begin();
        for (; it1 < A.data.end(); ++it1, ++it2, ++it3) {
            *it3 = std::min(*it1, *it2);
        }

        return ROIVertMat3D<T>(mindata, A.nframes, A.nrows, A.ncols);
    }
    ROIVertMat3D<T> operator+ (const ROIVertMat3D<T>& right) {
        Q_ASSERT(data.size() == right.data.size() && 
            nrows == right.nrows &&
            ncols == right.ncols);

        std::vector<double> sumdata(data.size());
        auto it1 = data.begin();
        auto it2 = right.data.begin();
        auto it3 = sumdata.begin();
        for (; it1 < data.end(); ++it1, ++it2, ++it3) {
            *it3 = *it1 + *it2;
        }

        return ROIVertMat3D<T>(sumdata, nframes, nrows, ncols);
    }
    ROIVertMat3D<T> operator- (const ROIVertMat3D<T>& right) {

        Q_ASSERT((data.size() == right.data.size() || right.nframes == 1)&& 
            nrows == right.nrows &&
            ncols == right.ncols);
        
        std::vector<double> sumdata(data.size());
        bool expand = right.nframes == 1 && nframes > 1;

        auto it1 = data.begin();
        auto it2 = right.data.begin();
        auto it3 = sumdata.begin();
        
        for (; it1 < data.end(); ++it1, ++it2, ++it3) {
            if (expand && it2 == right.data.end()) {
                it2 = right.data.begin();
            }
            *it3 = *it1 - *it2;
            
        }
        
        return ROIVertMat3D<T>(sumdata, nframes, nrows, ncols);
    }
    ROIVertMat3D<T> operator/ (const ROIVertMat3D<T>& right) {
        Q_ASSERT((data.size() == right.data.size() || right.nframes == 1)&& 
            nrows == right.nrows &&
            ncols == right.ncols);
        bool expand = right.nframes == 1 && nframes > 1;

        std::vector<double> sumdata(data.size());
        auto it1 = data.begin();
        auto it2 = right.data.begin();
        auto it3 = sumdata.begin();
        for (; it1 < data.end(); ++it1, ++it2, ++it3) {
            if (expand && it2 == right.data.end()) {
                it2 = right.data.begin();
            }
            *it3 = *it1 / *it2;
        }
        return ROIVertMat3D<T>(sumdata, nframes, nrows, ncols);
    }
    ROIVertMat3D<T> operator/ (const T& right) {

        std::vector<double> divdata(data.size());
        auto it1 = data.begin();
        auto it2 = divdata.begin();
        for (; it1 < data.end(); ++it1, ++it2) {
            *it2 = *it1 / right;
        }
        return ROIVertMat3D<T>(divdata, nframes, nrows, ncols);
    }
    ROIVertMat3D<T> operator- (const T& right) {

        std::vector<double> divdata(data.size());
        auto it1 = data.begin();
        auto it2 = divdata.begin();
        for (; it1 < data.end(); ++it1, ++it2) {
            *it2 = *it1 - right;
        }
        return ROIVertMat3D<T>(divdata, nframes, nrows, ncols);
    }
    ROIVertMat3D<T> operator* (const T& right) {

        std::vector<double> divdata(data.size());
        auto it1 = data.begin();
        auto it2 = divdata.begin();
        for (; it1 < data.end(); ++it1, ++it2) {
            *it2 = *it1 * right;
        }
        return ROIVertMat3D<T>(divdata, nframes, nrows, ncols);
    }
    bool operator==(const ROIVertMat3D<T>& rhs){ 
        if (data.size() != rhs.data.size() ||
            nrows != rhs.nrows || ncols != rhs.ncols) {
            return false;
        }
        for (size_t i = 0; i < data.size(); ++i) {
            if (data[i] != rhs.data[i]) { 
                return false 
            };
        }
        return true;
    }
    static bool almostequal(const ROIVertMat3D<uint8_t>& lhs, const ROIVertMat3D<uint8_t>& rhs, uint8_t tol) {
        // test for within 1 equality to deal with rounding strategy by opencv(?)
        
        if (lhs.data.size() != rhs.data.size() ||
            lhs.nrows != rhs.nrows || lhs.ncols != rhs.ncols) {
            return false;
        }
        for (size_t i = 0; i < lhs.data.size(); ++i) {
            if (std::abs(lhs.data[i] - rhs.data[i]) > tol) { 
                qDebug() << lhs.data[i] << rhs.data[i];
                return false;
            };
        }
        return true;
    }

    T globalMin() {
        Q_ASSERT(data.size() > 0);

        T ret = data[0];
        for (auto& val : data) {
            ret = std::min(ret, val);
        }
        return ret;
    }
    T globalMax() {
        Q_ASSERT(data.size() > 0);

        T ret = data[0];
        for (auto& val : data) {
            ret = std::max(ret, val);
        }
        return ret;
    }
    operator QString() const 
    {
        QString os;
        size_t cntr = 0;
        for (auto val : data) {
            if (cntr % nrows == 0) {
                os += "\n";
            }
            if (cntr % (nrows  * ncols) == 0) {
                os += "\n";
            }
            os += QString::number(val) + " ";
            ++cntr;
        }
        return os;

    }
    ROIVertMat3D<T> getMaxProjection() {
        if (data.empty()) {
            return ROIVertMat3D<T>();
        }

        auto fr = getSlice(0);
        for (size_t i = 1; i < nframes; ++i) {
            fr = ROIVertMat3D<T>::max(fr, getSlice(i));
        }
        return fr;
    }
    ROIVertMat3D<T> getMinProjection() {
        if (data.empty()) {
            return ROIVertMat3D<T>();
        }

        auto fr = getSlice(0);
        for (size_t i = 1; i < nframes; ++i) {
            fr = ROIVertMat3D<T>::min(fr, getSlice(i));
        }
        return fr;
    }

    ROIVertMat3D<T> getMeanProjection() {
        if (data.empty()) {
            return ROIVertMat3D<T>();
        }
        auto meandouble = getSumProjection() / nframes;
        return meandouble.cast<T>();
    }
    ROIVertMat3D<double> getSumProjection() {
        std::vector<double> sumvec(nrows*ncols, 0);
        ROIVertMat3D<double> sumprojection(sumvec, 1, nrows, ncols);
        
        for (size_t i = 0; i < nframes; ++i) {
            sumprojection = sumprojection + getSlice(i).cast<double>();
        }
        return sumprojection;
    }
    template<class TT> ROIVertMat3D<TT> cast() {
        // cast the data
        // create the new thing
        std::vector<TT> newdata;
        newdata.reserve(data.size());
        for (auto& val : data) {
            newdata.push_back(val); 
        }
        return ROIVertMat3D<TT>(newdata, nframes, nrows, ncols);
    }

private:
    size_t nrows = 0;
    size_t ncols = 0;
    size_t nframes = 0;
    std::vector<T> data;
};


//void generatedatasets();
void loaddataset(VideoData* data, datasettype = datasettype::ONESTACK, double framerate = 10., int downspace = 1, int downtime = 1);
template<class Ta, class Tb> inline bool nearlyequal(Ta a, Tb b, double eps = EPS){ return abs(a - b) < eps; }
template<class Ta, class Tb> inline bool nearlyequal(std::vector<Ta> a, std::vector<Tb> b, double eps = EPS){ 
    if (a.size() != b.size())
    {
        return false;
    }
        
    auto it1 = a.begin();
    auto it2 = b.begin();
    for (; it1 < a.end(); ++it1, ++it2) {
        if (!nearlyequal(*it1, *it2, eps)) {
            return false;
        }
    }
    return true;
}
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