#include <iostream>
#include "mythread.h"
#include <math.h>

MyThread::MyThread()
{
    m_list = new double*[N];
    for (int i = 0; i < N; i++)
        m_list[i] = new double[N];
}

MyThread::~MyThread()
{
    delete []m_list;
}

void MyThread::run()
{
    m_det = determinant(m_list, N);
}

void MyThread::setData(int num)
{
    m_num = num;

    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            m_list[j][i] = (j == m_num && m_num >= 0) ? b[i] : list[j][i];
}

double MyThread::getDet()
{
    return m_det;
}

double MyThread::determinant(double **matrix, int N)
{
    int i, j;
    double **minor;
    double out = 0;

    if (N == 2)
        out = matrix[0][0] * matrix[1][1] - matrix[0][1] * matrix[1][0];
    else
    {
        minor = new double*[N-1];

        for (i = 0; i < N; i++)
        {
            for (j = 0; j < N-1; j++)
                minor[j] = (j < i) ? matrix[j] : matrix[j+1];
            out += pow(-1, (i+j)) * determinant(minor, N-1) * matrix[i][N-1];
        }
        delete minor;
    }

    return out;
}
