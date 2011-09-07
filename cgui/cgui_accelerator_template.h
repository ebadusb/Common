/*
	Accelerator class that increases or decrease a given value over a period of time.
		This is a template class for any number type. The constructor sets up the frequency
		of  when the value of the is updated and the frequency of when the increment
		( or decrement if it is negative ) is updated. It also set is the _initIncrement, the _maxIncrement,
		the limits for the _value.  The startAccelerator() and stopAccelerator() methods will 
		start and stop the accelerator.  The startAccelerator() must have the initial value to start 
		accelerate and the stopAccelerator() return the _value of when the accelerator is stopped.
		The getCurrentValue() is where the owner of this object gets the accelerated value while the
		accelerator is running to update the display or media or value that needs the accelertor.
		If the _value to be accelerated needs to be changed before the timer updates, the owner will need
		to update the _value it passes in startsAccelerator().
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

	// Returns the current _value of the accelerated object.
	T getCurrentValue( void );

	// Starts the accelerator or changing _value.
	void startAccelerator( T currentValue );

	// Stops the accelerator or changing the value and returns the current _value.
	T stopAccelerator( void );

	// Returns true if the value has been updated. This is useful if the accelerator is returning
	// a delta that is no longer changing but whose value still needs to be sent periodically.
	bool valueUpdated(){return _valueUpdated;};

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
	bool _valueUpdated;
};

#endif /* ifndef _CGUI_ACCELERATOR_TEMPLATE_INCLUDE */

template< class T >
CGUIAccelerator<T>::CGUIAccelerator( T minLimit, T maxLimit, unsigned int valueUpdateFrequencyMS, unsigned int incrementUpdateFrequencyMS, T initIncrement, T maxIncrement ):
											_minLimit( minLimit ), _maxLimit( maxLimit ), _valueUpdateFrequencyMS( valueUpdateFrequencyMS ), _incrementUpdateFrequencyMS( incrementUpdateFrequencyMS ),
											_initIncrement( initIncrement ), _currIncrement( initIncrement ), _maxIncrement( maxIncrement ),_valueUpdated(false)
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

		osTime::snapshotRawTime( _updateIncrementTime );		
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
		osTime::snapshotRawTime( _updateValueTime );
		_valueUpdated=true;
	}
	else
	{
		_valueUpdated=false;
	}
	
	return _value;
}

template< class T >
void CGUIAccelerator<T>::startAccelerator( T currentValue )
{
	_value = currentValue;
	
	osTime::snapshotRawTime( _updateValueTime );		
	osTime::snapshotRawTime( _updateIncrementTime );
	_valueUpdated=false;
}

template< class T >
T CGUIAccelerator<T>::stopAccelerator( void )
{
	// Set back to initial value.
	_currIncrement = _initIncrement;
	_valueUpdated=false;
	
	return( _value );
}

