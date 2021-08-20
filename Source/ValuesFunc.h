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
    FuncPoint();
    ~FuncPoint();

    FuncPoint(T X, T Y);
    FuncPoint(const FuncPoint<T>& item);


    T x;
    T y;

private:

};


class ValuesFunc
{
public:
    ValuesFunc();
    ~ValuesFunc();
    void AddPoint(double x, double y);
    void AddPoint(FuncPoint<double> point);
    double GetValue(double x);
    double GetValue2(double x);
    FuncPoint<double>& GetValueAt(int index);
    int GetSize() { return values.size(); }
    void Resort();
    void SetAutoResort(bool value) { autoResortEnable = value; }
    bool GetAutoResort(bool value) { return autoResortEnable; }

    friend ValuesFunc* operator+(ValuesFunc& left, ValuesFunc& right);
    friend ValuesFunc* operator*(ValuesFunc& left, const double& right);
    friend ValuesFunc* operator*(const double& left, ValuesFunc& right);

    friend ValuesFunc* operator-(ValuesFunc& left, ValuesFunc& right);
    friend ValuesFunc* operator/(ValuesFunc& left, double& right);

    double GetAvg();
    double GetStandardDeviation();
    double GetFuncTreshold();
    void LimitingTheScope(double stDev, double koef = 1.0);

    FuncPoint<double>& GetMax();


private:
    std::vector<FuncPoint<double>> values;
    bool autoResortEnable{ false };
};


