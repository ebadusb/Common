/*
	Accelerator class that increases or decrease a given value over a period of time.
*/

#ifndef _CGUI_ACCELERATOR_TEMPLATE_INCLUDE
#define _CGUI_ACCELERATOR_TEMPLATE_INCLUDE

#include "ostime.hpp"

template< class T >

class CGUIAccelerator
{
public:
	CGUIAccelerator( T minLimit, T maxLimit, unsigned int valueUpdateFrequencyMS, unsigned int incrementUpdateFrequencyMS, T initIncrement, T maxIncrement );
	~CGUIAccelerator();

	T getCurrentValue( void );

	void startAccelerator( T currentValue );
	T stopAccelerator( void );

protected:
	rawTime _updateValueTime; //Last value update
	rawTime _updateIncrementTime;	// Last increment update
	
	T _value;

	T _minLimit;
	T _maxLimit;

	unsigned int _valueUpdateFrequencyMS; // How often value updated.
	unsigned int _incrementUpdateFrequencyMS;	// How often increment is updated.

	T _initIncrement;
	T _currIncrement;
	T _maxIncrement;

private:
	CGUIAccelerator();
	CGUIAccelerator ( const CGUIAccelerator &obj );
	CGUIAccelerator operator=( CGUIAccelerator &obj );
};

#endif /* ifndef _CGUI_ACCELERATOR_TEMPLATE_INCLUDE */

#include "

template< class T >
CGUIAccelerator<T>::CGUIAccelerator( T minLimit, T maxLimit, unsigned int valueUpdateFrequencyMS, unsigned int incrementUpdateFrequencyMS, T initIncrement, T maxIncrement ):
											_minLimit( minLimit ), _maxLimit( maxLimit ), _valueUpdateFrequencyMS( valueUpdateFrequencyMS ), _incrementUpdateFrequencyMS( incrementUpdateFrequencyMS ),
											_initIncrement( initIncrement ), _currIncrement( initIncrement ), _maxIncrement( maxIncrement )
{

	osTime::snapshotRawTime( _updateValueTime );		
	osTime::snapshotRawTime( _updateIncrementTime );		
}

template< class T >
CGUIAccelerator<T>::~CGUIAccelerator()
{
}

template< class T >
T CGUIAccelerator<T>::getCurrentValue( void )
{
	if( osTime::howLongMilliSec( _updateIncrementTime ) >= _incrementUpdateFrequencyMS )
	{
		_currIncrement = _currIncrement * 2;

		// Increasing
		if( _currIncrement > _maxIncrement  && _initIncrement <= _maxIncrement )
			_currIncrement = _maxIncrement;

		// Decreasing
		if( _currIncrement < _maxIncrement  && _initIncrement > _maxIncrement )
			_currIncrement = _maxIncrement;
	}	

	if( osTime::howLongMilliSec( _updateValueTime ) >= _valueUpdateFrequencyMS )
	{
		// Check limit based increment and change _value
		// 
		// Increasing
		if( _initIncrement <= _maxIncrement )
		{
			if( ( _value + _currIncrement ) <= _maxLimit )
			{
				_value += _currIncrement;
			}
			else
			{
				_value = _maxLimit;
			}
		}
		
		// Decreasing
		if( _initIncrement > _maxIncrement )
		{
			if( ( _value + _currIncrement ) >= _minLimit )
			{
				_value += _currIncrement;
			}
			else
			{
				_value = _minLimit;
			}
		}
	}
	
	return _value;
}

template< class T >
void CGUIAccelerator<T>::startAccelerator( T currentValue )
{
	_value = currentValue;
	
	osTime::snapshotRawTime( _updateValueTime );		
	osTime::snapshotRawTime( _updateIncrementTime );
}

template< class T >
T CGUIAccelerator<T>::stopAccelerator( void )
{
	// Set back to initial value.
	_currIncrement = _initIncrement;
	
	return( _value );
}

