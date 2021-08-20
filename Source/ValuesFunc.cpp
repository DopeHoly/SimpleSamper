/*
  ==============================================================================

    ValuesFunc.cpp
    Created: 6 May 2021 5:19:50pm
    Author:  menin

  ==============================================================================
*/

#include "ValuesFunc.h"
#include "MyPeakFinder.h"

template<typename T>
FuncPoint<T>::FuncPoint():
    x(0),
    y(0)
{
}

template<typename T>
FuncPoint<T>::~FuncPoint()
{
}

template<typename T>
FuncPoint<T>::FuncPoint(T X, T Y)
{
    x = X;
    y = Y;
}

template<typename T>
FuncPoint<T>::FuncPoint(const FuncPoint<T>& item)
{
    x = item.x;
    y = item.y;
}



bool PointComparer(FuncPoint<double> a, FuncPoint<double> b) {
    return a.x < b.x;
}



ValuesFunc::ValuesFunc():
    values()
{
}

ValuesFunc::~ValuesFunc()
{
    values.clear();
    values.shrink_to_fit();
}


void ValuesFunc::AddPoint(double x, double y)
{
    values.push_back(FuncPoint<double>(x, y));
    if (autoResortEnable) Resort();
}

void ValuesFunc::AddPoint(FuncPoint<double> point)
{
    values.push_back(point);
    if (autoResortEnable) Resort();
}

void ValuesFunc::Resort() 
{
    std::sort(values.begin(), values.end(), PointComparer);
}

double ValuesFunc::GetAvg()
{
    double sum = 0;
    for each (auto item in values)
    {
        sum += item.y;
    }
    int size = values.size();
    double avg = sum / double(size);
    return avg;
}

double ValuesFunc::GetStandardDeviation()
{
    double sum = 0;
    double avg = GetAvg();
    for each (auto item in values)
    {
        sum += pow(item.y - avg, 2);
    }

    int size = values.size();
    double stDevSqr = sum / double(size);
    double stDev = sqrt(stDevSqr);
    return stDev;
}

double ValuesFunc::GetFuncTreshold()
{    
    std::vector<float> out;
    for each (auto item in values)
    {
        out.push_back(item.y);
    }
    auto treshold = GetTreshold<float>(out);
    return treshold;
}

void ValuesFunc::LimitingTheScope(double stDev, double koef)
{
    int size = values.size();
    double treshold = stDev * koef;
    for (int i = 0; i < size; i++) {
        FuncPoint<double>& item = values.at(i);
        if (item.y <= treshold) {
            item.y = 0;
        }
    }
}

FuncPoint<double>& ValuesFunc::GetMax()
{
    double max = -INFINITY;
    int maxIndex = -1;
    int counter = 0;
    for each (auto item in values) {
        if (item.y > max) {
            max = item.y;
            maxIndex = counter;
        }
        ++counter;
    }
    return values.at(maxIndex);
}

//
//double ValuesFunc::GetValue(double x)
//{
//    //отбрасывание значений вне диапозона
//    if (x < values.front().x) return 0;
//    if (x > values.back().x) return 0;
//
//    double xValue;
//    FuncPoint<double> a;
//    FuncPoint<double> b;
//
//    //поиск значений
//    for (auto it = values.begin(); it != values.end(); ++it)
//    {
//        xValue = (*it).x;
//
//        if (xValue == x) {
//            return (*it).y;
//        }
//
//        if (xValue < x) {
//            a = *it;
//        }
//        else {
//            b = *it;
//            break;
//        }
//    }
//
//    //линейна€ интерпол€ци€
//    double value = a.y + ((b.y - a.y) / (b.x - a.x)) * (x - a.x);
//
//    return value;
//}

//алгоритм поиска значени€ с помощью бинарного поиска
double ValuesFunc::GetValue(double x) {
    if (x < values.front().x) return 0;
    if (x > values.back().x) return 0;

    double xValue;
    FuncPoint<double> a;
    FuncPoint<double> b;

    int l = -1;
    int r = values.size();
    int middle = 0;
    while(l < r - 1){
        middle = (l + r) / 2;
        xValue = values.at(middle).x;
        if (xValue == x) {
            return values.at(middle).y;
        }

        if (xValue < x)
            l = middle;
        else
            r = middle;
    }
    a = values.at(l);
    b = values.at(r);
    //линейна€ интерпол€ци€
    double value = a.y + ((b.y - a.y) / (b.x - a.x)) * (x - a.x);

    return value;
}

double ValuesFunc::GetValue2(double x)
{
    //отбрасывание значений вне диапозона
    if (x < values.front().x) return 0;
    if (x > values.back().x) return 0;

    double step = values.at(1).x - values.at(0).x;
    
    auto ind = x / step;
    auto indexLeft = trunc(ind);
    auto ost = ind - indexLeft;

    if (ost != 0) {

        FuncPoint<double> a;
        FuncPoint<double> b;
        a = values.at(indexLeft);
        b = values.at(indexLeft + 1);
        double value = a.y + ((b.y - a.y) / (b.x - a.x)) * (x - a.x);

        return value;
    }
    else {
        return values.at(indexLeft).y;
    }
}

FuncPoint<double>& ValuesFunc::GetValueAt(int index)
{
    return values.at(index);
}

ValuesFunc* operator+(ValuesFunc& left, ValuesFunc& right)
{
    auto result = new ValuesFunc();
    ValuesFunc* maxFunc;
    ValuesFunc* minFunc;
    if (left.GetSize() > right.GetSize()) {
        maxFunc = &left;
        minFunc = &right;
    }
    else {
        minFunc = &left;
        maxFunc = &right;
    }
    int size = maxFunc->GetSize();
    double freq; 
    double value;
    for (int i = 0; i < size; i++) 
    {
        auto point = maxFunc->GetValueAt(i);
        freq = point.x;
        auto maxValueItem = point.y;
        auto minValueItem = minFunc->GetValue2(freq);

        if (maxValueItem == 0 || minValueItem == 0) {
            value = 0;
        }
        else {
            value = point.y + minFunc->GetValue2(freq);
        }
        result->AddPoint(freq, value);
    }
    return result;
}

ValuesFunc* operator*(ValuesFunc& left, const double& right)
{
    auto result = new ValuesFunc();

    int size = left.GetSize();
    double freq;
    double value;
    for (int i = 0; i < size; i++)
    {
        auto point = left.GetValueAt(i);
        freq = point.x;
        value = point.y * right;
        result->AddPoint(freq, value);
    }
    return result;
}

ValuesFunc* operator*(const double& left, ValuesFunc& right)
{
    return operator*(right, left);
}

ValuesFunc* operator-(ValuesFunc& left, ValuesFunc& right)
{
    return (left) + *(right * (-1));
}

ValuesFunc* operator/(ValuesFunc& left, double& right)
{
    return left * (1.0 / right);
}
