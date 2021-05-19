/*
  ==============================================================================

    ValuesFunc.h
    Created: 6 May 2021 5:19:50pm
    Author:  menin

  ==============================================================================
*/

#pragma once
#include <vector>
#include <algorithm>

template <typename T>
class FuncPoint
{
public:
    FuncPoint() {}
    ~FuncPoint() {}

    FuncPoint(T X, T Y) 
    {
        x = X;
        y = Y;
    }

    //Point(Point<T> point) 
    //{
    //    x = point.x;
    //    y = point.y;
    //}

    T x;
    T y;

private:

};

bool PointComparer(FuncPoint<double> a, FuncPoint<double> b) {
    return a.x < b.x;
}

class ValuesFunc
{
public:
    ValuesFunc();
    ~ValuesFunc();
    void AddPoint(double x, double y);
    void AddPoint(FuncPoint<double> point);
    double GetValue(double x);

private:
    std::vector<FuncPoint<double>> values;
};

ValuesFunc::ValuesFunc()
{
}

ValuesFunc::~ValuesFunc()
{
    values.shrink_to_fit();
}


void ValuesFunc::AddPoint(double x, double y) 
{
    values.push_back(FuncPoint<double>(x, y));
    std::sort(values.begin(), values.end(), PointComparer);
}

void ValuesFunc::AddPoint(FuncPoint<double> point)
{
    values.push_back(point);
    std::sort(values.begin(), values.end(), PointComparer);
}


double ValuesFunc::GetValue(double x)
{
    //îòáðàñûâàíèå çíà÷åíèé âíå äèàïîçîíà
    if (x < values.front().x) return 0;
    if (x > values.back().x) return 0;

    double xValue;
    FuncPoint<double> a;
    FuncPoint<double> b;

    //Ïîèñê çíà÷åíèÿ
    for (auto it = values.begin(); it != values.end(); ++it) 
    {
        xValue = (*it).x;
        
        if (xValue == x) {
            return (*it).y;
        }

        if (xValue < x) {
            a = *it;
        }
        else {
            b = *it;
            break;
        }
    }

    //ëèíåéíàÿ èíòåðïîëÿöèÿ
    double value = a.y + ((b.y - a.y) / (b.x - a.x)) * (x - a.x);

    return value;
}

