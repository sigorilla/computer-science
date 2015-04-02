/*
 * Usage for test file:
 * -
 * N
 * A
 * b
 * M
 * -
 * You can enter your matrix in one line
  */

#include <iostream>
#include <QThread>
#include <QDebug>
#include <QList>
#include <QFile>
#include <QChar>
#include <QString>
#include <QStringList>
#include <QRegExp>
#include "mythread.h"

int N, M;
double ** list, * b;

int main( int argc, char ** argv ) {
	int k = 0;
	// Input data
	QString fileName = "D://mipt/inform/qt/cramer/test.txt";

	if ( fileName != "" ) {
		QFile file( fileName );
		if ( !file.open( QIODevice::ReadOnly ) ) {
			std::cout << "Could not open file";
			return 0;
		}
		QString contents = file.readAll().constData();
		QStringList strList = contents.split( QRegExp( "\\s+|\\n" ) );
		qDebug() << strList;

		N = strList[ k++ ].toInt();
		std::cout << "Your size of your matrix:\n\t N = ";
		std::cout << N << std::endl;
		if ( N <= 0 ) {
			std::cout << "N cannot be zero or negative...\nCheck your data.";
			return 0;
		}

		if ( N == 1 ) {
			double a, c;
			std::cout << "Your matrix \n";
			a = strList[ k++ ].toDouble();
			std::cout << a;
			std::cout << "Your column \n";
			c = strList[ k++ ].toDouble();
			std::cout << c;
			std::cout << "x[ 1 ] = " << (c/a) << "\n";
			std::cout << "END!\n";
			return 0;
		}

		list = new double * [ N ];
		for ( int j = 0; j < N; j++ ) {
			list[ j ] = new double[ N ];
		}
		b = new double[ N ];

		for ( int i = 0; i < N; i++ ) {
			for ( int j = 0; j < N; j++ ) {
				list[ j ][ i ] = strList[ k++ ].toDouble();
			}
		}

		for ( int i = 0; i < N; i++ ) {
			b[ i ] = strList[ k++ ].toDouble();
		}

		std::cout << "Check your set of equations\n";
		std::cout << "===================================\n";
		for ( int i = 0; i < N; i++ ) {
			for ( int j = 0; j < N; j++ ) {
				std::cout << " " << list[ j ][ i ] << " ";
			}
			std::cout << " | " << b[ i ] << "\n";
		}
		std::cout << "===================================\n\n";

		std::cout << "Your number of threads:\n\tM = ";
		M = strList[ k++ ].toInt();
		std::cout << M << std::endl;
		if ( M <= 0 ) {
			std::cout << "M cannot be zero or negative...\nCheck your data.";
			return 0;
		}
		file.close();
	}

	// Decision

	QList<QThread *> l;
	MyThread *t = new MyThread[ M ];

	MyThread tm;
	tm.setData( -1 );
	l << &tm;

	int All = N, to = 0, from = 0;
	while ( All > 0 ) {
		to = ( All <= M ) ? All : M;
		for ( int i = 0; i < to; i++ ) {
			t[ i ].setData( from );
			l << &t[ i ];
			from++;
		}
		foreach ( QThread * t, l ) {
			t->start();
		}
		All = All - to;
		foreach ( QThread * t, l ) {
			t->wait();
		}
	}

	// Results

	if ( tm.getDet() == 0 ) {
		std::cout << "No solutions. Main determinant is zero.\n";
		std::cout << "END!\n";
		return 0;
	}

	std::cout << "===================================\n";
	std::cout << "Solution:\n\n";
	for ( int j = 0; j < N; j++ ) {
		double det = t[ j ].getDet();
		std::cout << "\tx[" << ( j + 1 ) << "] = ";
		std::cout << ( ( det == 0.0 ) ? det : ( det / tm.getDet() ) );
		std::cout << "\n";
	}
	std::cout << "===================================\n";

	delete [] list;
	delete b;
	delete t;

	std::cout << "END!\n";

	return 0;
}
