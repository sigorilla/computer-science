#ifndef MYTHREAD_H
#define MYTHREAD_H

#include <QThread>

extern int N;
extern double ** list, * b;

class MyThread : public QThread {
	private:
		int m_num;
		double m_det;
		double ** m_list;

	public:
		MyThread();
		~MyThread();
		virtual void run();
		void setData( int );
		double getDet();
		double determinant( double **, int );
};

#endif // MYTHREAD_H
