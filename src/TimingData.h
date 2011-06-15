#ifndef TIMING_DATA_H
#define TIMING_DATA_H

#include "NoteTypes.h"
#include "TimingSegments.h"
#include "PrefsManager.h"
struct lua_State;

/** @brief Compare a TimingData segment's properties with one another. */
#define COMPARE(x) if(x!=other.x) return false;

/**
 * @brief Holds data for translating beats<->seconds.
 */
class TimingData
{
public:
	/**
	 * @brief Sets up initial timing data.
	 */
	TimingData();
	/**
	 * @brief Sets up initial timing data with a defined offset.
	 * @param fOffset the offset from the 0th beat. */
	TimingData(float fOffset);
	/**
	 * @brief Gets the actual BPM of the song.
	 * @param fMinBPMOut the minimium specified BPM.
	 * @param fMaxBPMOut the maximum specified BPM.
	 */
	void GetActualBPM( float &fMinBPMOut, float &fMaxBPMOut ) const;
	/**
	 * @brief Retrieve the BPM at the given row.
	 * @param iNoteRow the row in question.
	 * @return the BPM.
	 */
	float GetBPMAtRow( int iNoteRow ) const;
	/**
	 * @brief Retrieve the BPM at the given beat.
	 * @param fBeat the beat in question.
	 * @return the BPM.
	 */
	float GetBPMAtBeat( float fBeat ) const { return GetBPMAtRow( BeatToNoteRow(fBeat)); }
	/**
	 * @brief Set the row to have the new BPM.
	 * @param iNoteRow the row to have the new BPM.
	 * @param fBPM the BPM.
	 */
	void SetBPMAtRow( int iNoteRow, float fBPM );
	/**
	 * @brief Set the beat to have the new BPM.
	 * @param fBeat the beat to have the new BPM.
	 * @param fBPM the BPM.
	 */
	void SetBPMAtBeat( float fBeat, float fBPM ) { SetBPMAtRow( BeatToNoteRow(fBeat), fBPM ); }
	/**
	 * @brief Retrieve the BPMSegment at the specified row.
	 * @param iNoteRow the row that has a BPMSegment.
	 * @return the BPMSegment in question.
	 */
	BPMSegment& GetBPMSegmentAtRow( int iNoteRow );
	/**
	 * @brief Retrieve the BPMSegment at the specified beat.
	 * @param fBeat the beat that has a BPMSegment.
	 * @return the BPMSegment in question.
	 */
	BPMSegment& GetBPMSegmentAtBeat( float fBeat ) { return GetBPMSegmentAtRow( (int)BeatToNoteRow(fBeat)); }
	/**
	 * @brief Retrieve the index of the BPMSegments at the specified row.
	 * @param iNoteRow the row that has a BPMSegment.
	 * @return the BPMSegment's index in question.
	 */
	int GetBPMSegmentIndexAtRow( int iNoteRow ) const;
	/**
	 * @brief Retrieve the index of the BPMSegments at the specified beat.
	 * @param fBeat the beat that has a BPMSegment.
	 * @return the BPMSegment's index in question.
	 */
	int GetBPMSegmentIndexAtBeat( float fBeat ) const { return GetBPMSegmentIndexAtRow( BeatToNoteRow(fBeat)); }
	/**
	 * @brief Add the BPMSegment to the TimingData.
	 * @param seg the new BPMSegment.
	 */
	void AddBPMSegment( const BPMSegment &seg );
	
	/**
	 * @brief Retrieve the Stop/Delay at the given row.
	 * @param iNoteRow the row in question.
	 * @param bDelayOut A flag to determine if we are getting a delay or not.
	 * @return the time we stop at this row.
	 */
	float GetStopAtRow( int iNoteRow, bool bDelayOut ) const;
	/**
	 * @brief Retrieve the Stop/Delay at the given row.
	 * @param fBeat the beat in question.
	 * @param bDelayOut A flag to determine if we are getting a delay or not.
	 * @return the time we stop at this beat.
	 */
	float GetStopAtBeat( float fBeat, bool bDelayOut ) const { return GetStopAtRow( BeatToNoteRow(fBeat), bDelayOut ); }
	/**
	 * @brief Retrieve the stop time at the given row.
	 * @param iNoteRow the row in question.
	 * @return the stop time.
	 */
	float GetStopAtRow( int iNoteRow ) const;
	/**
	 * @brief Retrieve the stop time at the given beat.
	 * @param fBeat the beat in question.
	 * @return the stop time.
	 */
	float GetStopAtBeat( float fBeat ) const { return GetStopAtRow( BeatToNoteRow(fBeat) ); }
	/**
	 * @brief Retrieve the delay time at the given row.
	 * @param iNoteRow the row in question.
	 * @return the delay time.
	 */
	float GetDelayAtRow( int iNoteRow ) const;
	/**
	 * @brief Retrieve the delay time at the given beat.
	 * @param fBeat the beat in question.
	 * @return the delay time.
	 */
	float GetDelayAtBeat( float fBeat ) const { return GetDelayAtRow( BeatToNoteRow(fBeat) ); }
	/**
	 * @brief Set the row to have the new stop time.
	 * @param iNoteRow the row to have the new stop time.
	 * @param fSeconds the new stop time.
	 */
	void SetStopAtRow( int iNoteRow, float fSeconds ) { SetStopAtRow( iNoteRow, fSeconds, false ); }
	/**
	 * @brief Set the row to have the new pause time.
	 *
	 * This function was added specifically for sm-ssc.
	 * @param iNoteRow the row to have the new pause time.
	 * @param fSeconds the new pause time.
	 * @param bDelay If true, this is a Delay Segment. Otherwise, it is a StopSegment.
	 */
	void SetStopAtRow( int iNoteRow, float fSeconds, bool bDelay );
	/**
	 * @brief Set the row to have the new delay time.
	 *
	 * This function was added specifically for sm-ssc.
	 * @param iNoteRow the row to have the new delay time.
	 * @param fSeconds the new delay time.
	 */
	void SetDelayAtRow( int iNoteRow, float fSeconds ) { SetStopAtRow( iNoteRow, fSeconds, true ); }
	/**
	 * @brief Set the beat to have the new stop time.
	 * @param fBeat to have the new stop time.
	 * @param fSeconds the new stop time.
	 */
	void SetStopAtBeat( float fBeat, float fSeconds ) { SetStopAtRow( BeatToNoteRow(fBeat), fSeconds, false ); }
	/**
	 * @brief Set the beat to have the new pause time.
	 *
	 * This function was added specifically for sm-ssc.
	 * @param fBeat the beat to have the new pause time.
	 * @param fSeconds the new pause time.
	 * @param bDelay If true, this is a Delay Segment. Otherwise, it is a StopSegment.
	 */
	void SetStopAtBeat( float fBeat, float fSeconds, bool bDelay ) { SetStopAtRow( BeatToNoteRow(fBeat), fSeconds, bDelay ); }
	/**
	 * @brief Set the beat to have the new delay time.
	 *
	 * This function was added specifically for sm-ssc.
	 * @param fBeat the beat to have the new delay time.
	 * @param fSeconds the new delay time.
	 */
	void SetDelayAtBeat( float fBeat, float fSeconds ) { SetStopAtRow( BeatToNoteRow(fBeat), fSeconds, true ); }
	/**
	 * @brief Retrieve the StopSegment at the specified row.
	 * @param iNoteRow the row that has a StopSegment.
	 * @return the StopSegment in question.
	 */
	StopSegment& GetStopSegmentAtRow( int iNoteRow ) { return GetStopSegmentAtRow( iNoteRow, false ); }
	/**
	 * @brief Retrieve the StopSegment at the specified beat.
	 * @param fBeat the beat that has a StopSegment.
	 * @return the StopSegment in question.
	 */
	StopSegment& GetStopSegmentAtBeat( float fBeat ) { return GetStopSegmentAtRow( BeatToNoteRow(fBeat), false); }
	/**
	 * @brief Retrieve the StopSegment at the specified row.
	 * @param iNoteRow the row that has a StopSegment.
	 * @param bDelay If true, this is actually a DelaySegment.
	 * @return the StopSegment in question.
	 */
	StopSegment& GetStopSegmentAtRow( int iNoteRow, bool bDelay );
	/**
	 * @brief Retrieve the StopSegment at the specified beat.
	 * @param fBeat the beat that has a StopSegment.
	 * @param bDelay If true, this is actually a DelaySegment.
	 * @return the StopSegment in question.
	 */
	StopSegment& GetStopSegmentAtBeat( float fBeat, bool bDelay ) { return GetStopSegmentAtRow( BeatToNoteRow(fBeat), bDelay ); }
	/**
	 * @brief Retrieve the DelaySegment at the specified row.
	 * @param iNoteRow the row that has a DelaySegment.
	 * @return the DelaySegment in question.
	 */
	StopSegment& GetDelaySegmentAtRow( int iNoteRow ) { return GetStopSegmentAtRow( iNoteRow, true ); }
	/**
	 * @brief Retrieve the DelaySegment at the specified beat.
	 * @param fBeat the beat that has a DelaySegment.
	 * @return the DelaySegment in question.
	 */
	StopSegment& GetDelaySegmentAtBeat( float fBeat ) { return GetStopSegmentAtRow( BeatToNoteRow(fBeat), true); }
	/**
	 * @brief Retrieve the index of the StopSegments at the specified row.
	 * @param iNoteRow the row that has a StopSegment.
	 * @return the StopSegment's index in question.
	 */
	int GetStopSegmentIndexAtRow( int iNoteRow ) const { return GetStopSegmentIndexAtRow( iNoteRow, false ); }
	/**
	 * @brief Retrieve the index of the StopSegments at the specified beat.
	 * @param fBeat the beat that has a StopSegment.
	 * @return the StopSegment's index in question.
	 */
	int GetStopSegmentIndexAtBeat( float fBeat ) const { return GetStopSegmentIndexAtRow( BeatToNoteRow(fBeat), false ); }
	/**
	 * @brief Retrieve the index of the StopSegments at the specified row.
	 * @param iNoteRow the row that has a StopSegment.
	 * @param bDelay If true, it's a Delay Segment. Otherwise, it's a StopSegment.
	 * @return the StopSegment's index in question.
	 */
	int GetStopSegmentIndexAtRow( int iNoteRow, bool bDelay ) const;
	/**
	 * @brief Retrieve the index of the StopSegments at the specified beat.
	 * @param fBeat the beat that has a StopSegment.
	 * @param bDelay If true, it's a Delay Segment. Otherwise, it's a StopSegment.
	 * @return the StopSegment's index in question.
	 */
	int GetStopSegmentIndexAtBeat( float fBeat, bool bDelay ) const { return GetStopSegmentIndexAtRow( BeatToNoteRow(fBeat), bDelay ); }
	/**
	 * @brief Retrieve the index of the Delay Segments at the specified row.
	 * @param iNoteRow the row that has a Delay Segment.
	 * @return the StopSegment's index in question.
	 */
	int GetDelaySegmentIndexAtRow( int iNoteRow ) const { return GetStopSegmentIndexAtRow( iNoteRow, true ); }
	/**
	 * @brief Retrieve the index of the Delay Segments at the specified beat.
	 * @param fBeat the beat that has a Delay Segment.
	 * @return the StopSegment's index in question.
	 */
	int GetDelaySegmentIndexAtBeat( float fBeat ) const { return GetStopSegmentIndexAtRow( BeatToNoteRow(fBeat), true ); }
	/**
	 * @brief Add the StopSegment to the TimingData.
	 * @param seg the new StopSegment.
	 */
	void AddStopSegment( const StopSegment &seg );
	
	/**
	 * @brief Retrieve the Time Signature's numerator at the given row.
	 * @param iNoteRow the row in question.
	 * @return the numerator.
	 */
	int GetTimeSignatureNumeratorAtRow( int iNoteRow );
	/**
	 * @brief Retrieve the Time Signature's numerator at the given beat.
	 * @param fBeat the beat in question.
	 * @return the numerator.
	 */
	int GetTimeSignatureNumeratorAtBeat( float fBeat ) { return GetTimeSignatureNumeratorAtRow( BeatToNoteRow(fBeat) ); }
	/**
	 * @brief Retrieve the Time Signature's denominator at the given row.
	 * @param iNoteRow the row in question.
	 * @return the denominator.
	 */
	int GetTimeSignatureDenominatorAtRow( int iNoteRow );
 	/**
	 * @brief Retrieve the Time Signature's denominator at the given beat.
	 * @param fBeat the beat in question.
	 * @return the denominator.
	 */
	int GetTimeSignatureDenominatorAtBeat( float fBeat ) { return GetTimeSignatureDenominatorAtRow( BeatToNoteRow(fBeat) ); }
	/**
	 * @brief Set the row to have the new Time Signature.
	 * @param iNoteRow the row to have the new Time Signature.
	 * @param iNumerator the numerator.
	 * @param iDenominator the denominator.
	 */
	void SetTimeSignatureAtRow( int iNoteRow, int iNumerator, int iDenominator );
	/**
	 * @brief Set the beat to have the new Time Signature.
	 * @param fBeat the beat to have the new Time Signature.
	 * @param iNumerator the numerator.
	 * @param iDenominator the denominator.
	 */
	void SetTimeSignatureAtBeat( float fBeat, int iNumerator, int iDenominator ) { SetTimeSignatureAtRow( BeatToNoteRow(fBeat), iNumerator, iDenominator ); }
	/**
	 * @brief Set the row to have the new Time Signature numerator.
	 * @param iNoteRow the row to have the new Time Signature numerator.
	 * @param iNumerator the numerator.
	 */
	void SetTimeSignatureNumeratorAtRow( int iNoteRow, int iNumerator );
	/**
	 * @brief Set the beat to have the new Time Signature numerator.
	 * @param fBeat the beat to have the new Time Signature numerator.
	 * @param iNumerator the numerator.
	 */
	void SetTimeSignatureNumeratorAtBeat( float fBeat, int iNumerator ) { SetTimeSignatureNumeratorAtRow( BeatToNoteRow(fBeat), iNumerator); }
	/**
	 * @brief Set the row to have the new Time Signature denominator.
	 * @param iNoteRow the row to have the new Time Signature denominator.
	 * @param iDenominator the denominator.
	 */
	void SetTimeSignatureDenominatorAtRow( int iNoteRow, int iDenominator );
	/**
	 * @brief Set the beat to have the new Time Signature denominator.
	 * @param fBeat the beat to have the new Time Signature denominator.
	 * @param iDenominator the denominator.
	 */
	void SetTimeSignatureDenominatorAtBeat( float fBeat, int iDenominator ) { SetTimeSignatureDenominatorAtRow( BeatToNoteRow(fBeat), iDenominator); }
	/**
	 * @brief Retrieve the TimeSignatureSegment at the specified row.
	 * @param iNoteRow the row that has a TimeSignatureSegment.
	 * @return the TimeSignatureSegment in question.
	 */
	TimeSignatureSegment& GetTimeSignatureSegmentAtRow( int iNoteRow );
	/**
	 * @brief Retrieve the TimeSignatureSegment at the specified beat.
	 * @param fBeat the beat that has a TimeSignatureSegment.
	 * @return the TimeSignatureSegment in question.
	 */
	TimeSignatureSegment& GetTimeSignatureSegmentAtBeat( float fBeat ) { return GetTimeSignatureSegmentAtRow( BeatToNoteRow(fBeat) ); }
	/**
	 * @brief Retrieve the index of the TimeSignatureSegments at the specified row.
	 * @param iNoteRow the row that has a TimeSignatureSegment.
	 * @return the TimeSignatureSegment's index in question.
	 */
	int GetTimeSignatureSegmentIndexAtRow( int iNoteRow ) const;
	/**
	 * @brief Retrieve the index of the TimeSignatureSegments at the specified beat.
	 * @param fBeat the beat that has a TimeSignatureSegment.
	 * @return the TimeSignatureSegment's index in question.
	 */
	int GetTimeSignatureSegmentIndexAtBeat( float fBeat ) const { return GetTimeSignatureSegmentIndexAtRow( BeatToNoteRow(fBeat) ); }
	/**
	 * @brief Add the TimeSignatureSegment to the TimingData.
	 * @param seg the new TimeSignatureSegment.
	 */
	void AddTimeSignatureSegment( const TimeSignatureSegment &seg );
	/**
	 * @brief Determine the beat to warp to.
	 * @param iRow The row you start on.
	 * @return the beat you warp to.
	 */
	float GetWarpAtRow( int iRow ) const;
	/**
	 * @brief Determine the beat to warp to.
	 * @param fBeat The beat you start on.
	 * @return the beat you warp to.
	 */
	float GetWarpAtBeat( float fBeat ) const { return GetWarpAtRow( BeatToNoteRow( fBeat ) ); }
	/**
	 * @brief Set the beat to warp to given a starting row.
	 * @param iRow The row to start on.
	 * @param fNew The destination beat.
	 */
	void SetWarpAtRow( int iRow, float fNew );
	/**
	 * @brief Set the beat to warp to given a starting beat.
	 * @param fBeat The beat to start on.
	 * @param fNew The destination beat.
	 */
	void SetWarpAtBeat( float fBeat, float fNew ) { SetWarpAtRow( BeatToNoteRow( fBeat ), fNew ); }
	/**
	 * @brief Retrieve the WarpSegment at the specified row.
	 * @param iRow the row to focus on.
	 * @return the WarpSegment in question.
	 */
	WarpSegment& GetWarpSegmentAtRow( int iRow );
	/**
	 * @brief Retrieve the WarpSegment at the specified beat.
	 * @param fBeat the beat to focus on.
	 * @return the WarpSegment in question.
	 */
	WarpSegment& GetWarpSegmentAtBeat( float fBeat ) { return GetWarpSegmentAtRow( BeatToNoteRow( fBeat ) ); }
	/**
	 * @brief Retrieve the index of the WarpSegment at the specified row.
	 * @param iRow the row to focus on.
	 * @return the index in question.
	 */
	int GetWarpSegmentIndexAtRow( int iRow ) const;
	/**
	 * @brief Retrieve the index of the WarpSegment at the specified beat.
	 * @param fBeat the beat to focus on.
	 * @return the index in question.
	 */
	int GetWarpSegmentIndexAtBeat( float fBeat ) const { return GetWarpSegmentIndexAtRow( BeatToNoteRow( fBeat ) ); }
	/**
	 * @brief Checks if the row is inside a warp.
	 * @param iRow the row to focus on.
	 * @return true if the row is inside a warp, false otherwise.
	 */
	bool IsWarpAtRow( int iRow ) const;
	/**
	 * @brief Checks if the beat is inside a warp.
	 * @param fBeat the beat to focus on.
	 * @return true if the row is inside a warp, false otherwise.
	 */
	bool IsWarpAtBeat( float fBeat ) const { return IsWarpAtRow( BeatToNoteRow( fBeat ) ); }
	/**
	 * @brief Add the WarpSegment to the TimingData.
	 * @param seg the new WarpSegment.
	 */
	void AddWarpSegment( const WarpSegment &seg );
	/**
	 * @brief Retrieve the Tickcount at the given row.
	 * @param iNoteRow the row in question.
	 * @return the Tickcount.
	 */
	int GetTickcountAtRow( int iNoteRow ) const;
	/**
	 * @brief Retrieve the Tickcount at the given beat.
	 * @param fBeat the beat in question.
	 * @return the Tickcount.
	 */
	int GetTickcountAtBeat( float fBeat ) const { return GetTickcountAtRow( BeatToNoteRow(fBeat) ); }
	/**
	 * @brief Set the row to have the new tickcount.
	 * @param iNoteRow the row to have the new tickcount.
	 * @param iTicks the tickcount.
	 */
	void SetTickcountAtRow( int iNoteRow, int iTicks );
	/**
	 * @brief Set the beat to have the new tickcount.
	 * @param fBeat the beat to have the new tickcount.
	 * @param iTicks the tickcount.
	 */
	void SetTickcountAtBeat( float fBeat, int iTicks ) { SetTickcountAtRow( BeatToNoteRow( fBeat ), iTicks ); }
	/**
	 * @brief Retrieve the TickcountSegment at the specified row.
	 * @param iNoteRow the row that has a TickcountSegment.
	 * @return the TickcountSegment in question.
	 */
	TickcountSegment& GetTickcountSegmentAtRow( int iNoteRow );
	/**
	 * @brief Retrieve the TickcountSegment at the specified beat.
	 * @param fBeat the beat that has a TickcountSegment.
	 * @return the TickcountSegment in question.
	 */
	TickcountSegment& GetTickcountSegmentAtBeat( float fBeat ) { return GetTickcountSegmentAtRow( BeatToNoteRow(fBeat) ); }
	/**
	 * @brief Retrieve the index of the TickcountSegments at the specified row.
	 * @param iNoteRow the row that has a TickcountSegment.
	 * @return the TickcountSegment's index in question.
	 */
	int GetTickcountSegmentIndexAtRow( int iNoteRow ) const;
	/**
	 * @brief Retrieve the index of the TickcountSegments at the specified beat.
	 * @param fBeat the beat that has a TickcountSegment.
	 * @return the TickcountSegment's index in question.
	 */
	int GetTickcountSegmentIndexAtBeat( float fBeat ) const { return GetTickcountSegmentIndexAtRow( BeatToNoteRow(fBeat) ); }
	/**
	 * @brief Add the TickcountSegment to the TimingData.
	 * @param seg the new TickcountSegment.
	 */
	void AddTickcountSegment( const TickcountSegment &seg );
	
	/**
	 * @brief Retrieve the Combo at the given row.
	 * @param iNoteRow the row in question.
	 * @return the Combo.
	 */
	int GetComboAtRow( int iNoteRow ) const;
	/**
	 * @brief Retrieve the Combo at the given beat.
	 * @param fBeat the beat in question.
	 * @return the Combo.
	 */
	int GetComboAtBeat( float fBeat ) const { return GetComboAtRow( BeatToNoteRow(fBeat) ); }
	/**
	 * @brief Set the row to have the new Combo.
	 * @param iNoteRow the row to have the new Combo.
	 * @param iCombo the Combo.
	 */
	void SetComboAtRow( int iNoteRow, int iCombo );
	/**
	 * @brief Set the beat to have the new Combo.
	 * @param fBeat the beat to have the new Combo.
	 * @param iCombo the Combo.
	 */
	void SetComboAtBeat( float fBeat, int iCombo ) { SetComboAtRow( BeatToNoteRow( fBeat ), iCombo ); }
	/**
	 * @brief Retrieve the ComboSegment at the specified row.
	 * @param iNoteRow the row that has a ComboSegment.
	 * @return the ComboSegment in question.
	 */
	ComboSegment& GetComboSegmentAtRow( int iNoteRow );
	/**
	 * @brief Retrieve the ComboSegment at the specified beat.
	 * @param fBeat the beat that has a ComboSegment.
	 * @return the ComboSegment in question.
	 */
	ComboSegment& GetComboSegmentAtBeat( float fBeat ) { return GetComboSegmentAtRow( BeatToNoteRow(fBeat) ); }
	/**
	 * @brief Retrieve the index of the ComboSegments at the specified row.
	 * @param iNoteRow the row that has a ComboSegment.
	 * @return the ComboSegment's index in question.
	 */
	int GetComboSegmentIndexAtRow( int iNoteRow ) const;
	/**
	 * @brief Retrieve the index of the ComboSegments at the specified beat.
	 * @param fBeat the beat that has a ComboSegment.
	 * @return the ComboSegment's index in question.
	 */
	int GetComboSegmentIndexAtBeat( float fBeat ) const { return GetComboSegmentIndexAtRow( BeatToNoteRow(fBeat) ); }
	/**
	 * @brief Add the ComboSegment to the TimingData.
	 * @param seg the new ComboSegment.
	 */
	void AddComboSegment( const ComboSegment &seg );
	
	/**
	 * @brief Retrieve the Label at the given row.
	 * @param iNoteRow the row in question.
	 * @return the Label.
	 */
	RString GetLabelAtRow( int iNoteRow ) const;
	/**
	 * @brief Retrieve the Label at the given beat.
	 * @param fBeat the beat in question.
	 * @return the Label.
	 */
	RString GetLabelAtBeat( float fBeat ) const { return GetLabelAtRow( BeatToNoteRow(fBeat) ); }
	/**
	 * @brief Set the row to have the new Label.
	 * @param iNoteRow the row to have the new Label.
	 * @param sLabel the Label.
	 */
	void SetLabelAtRow( int iNoteRow, const RString sLabel );
	/**
	 * @brief Set the beat to have the new Label.
	 * @param fBeat the beat to have the new Label.
	 * @param sLabel the Label.
	 */
	void SetLabelAtBeat( float fBeat, const RString sLabel ) { SetLabelAtRow( BeatToNoteRow( fBeat ), sLabel ); }
	/**
	 * @brief Retrieve the LabelSegment at the specified row.
	 * @param iNoteRow the row that has a LabelSegment.
	 * @return the LabelSegment in question.
	 */
	LabelSegment& GetLabelSegmentAtRow( int iNoteRow );
	/**
	 * @brief Retrieve the LabelSegment at the specified beat.
	 * @param fBeat the beat that has a LabelSegment.
	 * @return the LabelSegment in question.
	 */
	LabelSegment& GetLabelSegmentAtBeat( float fBeat ) { return GetLabelSegmentAtRow( BeatToNoteRow(fBeat) ); }
	/**
	 * @brief Retrieve the index of the LabelSegments at the specified row.
	 * @param iNoteRow the row that has a LabelSegment.
	 * @return the LabelSegment's index in question.
	 */
	int GetLabelSegmentIndexAtRow( int iNoteRow ) const;
	/**
	 * @brief Retrieve the index of the LabelSegments at the specified beat.
	 * @param fBeat the beat that has a LabelSegment.
	 * @return the LabelSegment's index in question.
	 */
	int GetLabelSegmentIndexAtBeat( float fBeat ) const { return GetLabelSegmentIndexAtRow( BeatToNoteRow(fBeat) ); }
	/**
	 * @brief Add the LabelSegment to the TimingData.
	 * @param seg the new LabelSegment.
	 */
	void AddLabelSegment( const LabelSegment &seg );
	
	/**
	 * @brief Retrieve the previous beat that contains a LabelSegment.
	 * @param iRow the present row.
	 * @return the previous beat with a LabelSegment, or fBeat if there is none prior.
	 */
	float GetPreviousLabelSegmentBeatAtRow( int iRow ) const;
	/**
	 * @brief Retrieve the previous beat that contains a LabelSegment.
	 * @param fBeat the present beat.
	 * @return the previous beat with a LabelSegment, or fBeat if there is none prior.
	 */
	float GetPreviousLabelSegmentBeatAtBeat( float fBeat ) const { return GetPreviousLabelSegmentBeatAtRow( BeatToNoteRow(fBeat) ); }

	/**
	 * @brief Determine if the requisite label already exists.
	 * @param sLabel the label to check.
	 * @return true if it exists, false otherwise. */
	bool DoesLabelExist( RString sLabel ) const;
	
	/**
	 * @brief Retrieve the next beat that contains a LabelSegment.
	 * @param iRow the present row.
	 * @return the next beat with a LabelSegment, or fBeat if there is none ahead.
	 */
	float GetNextLabelSegmentBeatAtRow( int iRow ) const;
	/**
	 * @brief Retrieve the previous beat that contains a LabelSegment.
	 * @param fBeat the present beat.
	 * @return the next beat with a LabelSegment, or fBeat if there is none ahead.
	 */
	float GetNextLabelSegmentBeatAtBeat( float fBeat ) const { return GetNextLabelSegmentBeatAtRow( BeatToNoteRow(fBeat) ); }
	
	
	/**
	 * @brief Retrieve the Speed's percent at the given row.
	 * @param iNoteRow the row in question.
	 * @return the percent.
	 */
	float GetSpeedPercentAtRow( int iNoteRow );
	/**
	 * @brief Retrieve the Speed's percent at the given beat.
	 * @param fBeat the beat in question.
	 * @return the percent.
	 */
	float GetSpeedPercentAtBeat( float fBeat ) { return GetSpeedPercentAtRow( BeatToNoteRow(fBeat) ); }
	/**
	 * @brief Retrieve the Speed's wait at the given row.
	 * @param iNoteRow the row in question.
	 * @return the wait.
	 */
	float GetSpeedWaitAtRow( int iNoteRow );
 	/**
	 * @brief Retrieve the Speed's wait at the given beat.
	 * @param fBeat the beat in question.
	 * @return the wait.
	 */
	float GetSpeedWaitAtBeat( float fBeat ) { return GetSpeedWaitAtRow( BeatToNoteRow(fBeat) ); }
	/**
	 * @brief Retrieve the Speed's mode at the given row.
	 * @param iNoteRow the row in question.
	 * @return the mode.
	 */
	unsigned short GetSpeedModeAtRow( int iNoteRow );
 	/**
	 * @brief Retrieve the Speed's mode at the given beat.
	 * @param fBeat the beat in question.
	 * @return the mode.
	 */
	unsigned short GetSpeedModeAtBeat( float fBeat ) { return GetSpeedModeAtRow( BeatToNoteRow(fBeat) ); }
	/**
	 * @brief Set the row to have the new Speed.
	 * @param iNoteRow the row to have the new Speed.
	 * @param fPercent the percent.
	 * @param fWait the wait.
	 * @param usMode the mode.
	 */
	void SetSpeedAtRow( int iNoteRow, float fPercent, float fWait, unsigned short usMode );
	/**
	 * @brief Set the beat to have the new Speed.
	 * @param fBeat the beat to have the new Speed.
	 * @param fPercent the percent.
	 * @param fWait the wait.
	 * @param usMode the mode.
	 */
	void SetSpeedAtBeat( float fBeat, float fPercent, float fWait, unsigned short usMode ) { SetSpeedAtRow( BeatToNoteRow(fBeat), fPercent, fWait, usMode ); }
	/**
	 * @brief Set the row to have the new Speed percent.
	 * @param iNoteRow the row to have the new Speed percent.
	 * @param fPercent the percent.
	 */
	void SetSpeedPercentAtRow( int iNoteRow, float fPercent );
	/**
	 * @brief Set the beat to have the new Speed percent.
	 * @param fBeat the beat to have the new Speed percent.
	 * @param fPercent the percent.
	 */
	void SetSpeedPercentAtBeat( float fBeat, float fPercent ) { SetSpeedPercentAtRow( BeatToNoteRow(fBeat), fPercent); }
	/**
	 * @brief Set the row to have the new Speed wait.
	 * @param iNoteRow the row to have the new Speed wait.
	 * @param fWait the wait.
	 */
	void SetSpeedWaitAtRow( int iNoteRow, float fWait );
	/**
	 * @brief Set the beat to have the new Speed wait.
	 * @param fBeat the beat to have the new Speed wait.
	 * @param fWait the wait.
	 */
	void SetSpeedWaitAtBeat( float fBeat, float fWait ) { SetSpeedWaitAtRow( BeatToNoteRow(fBeat), fWait); }
	/**
	 * @brief Set the row to have the new Speed mode.
	 * @param iNoteRow the row to have the new Speed mode.
	 * @param usMode the mode.
	 */
	void SetSpeedModeAtRow( int iNoteRow, unsigned short usMode );
	/**
	 * @brief Set the beat to have the new Speed mode.
	 * @param fBeat the beat to have the new Speed mode.
	 * @param usMode the mode.
	 */
	void SetSpeedModeAtBeat( float fBeat, unsigned short usMode ) { SetSpeedModeAtRow( BeatToNoteRow(fBeat), usMode); }
	/**
	 * @brief Retrieve the SpeedSegment at the specified row.
	 * @param iNoteRow the row that has a SpeedSegment.
	 * @return the SpeedSegment in question.
	 */
	SpeedSegment& GetSpeedSegmentAtRow( int iNoteRow );
	/**
	 * @brief Retrieve the SpeedSegment at the specified beat.
	 * @param fBeat the beat that has a SpeedSegment.
	 * @return the SpeedSegment in question.
	 */
	SpeedSegment& GetSpeedSegmentAtBeat( float fBeat ) { return GetSpeedSegmentAtRow( BeatToNoteRow(fBeat) ); }
	/**
	 * @brief Retrieve the index of the SpeedSegments at the specified row.
	 * @param iNoteRow the row that has a SpeedSegment.
	 * @return the SpeedSegment's index in question.
	 */
	int GetSpeedSegmentIndexAtRow( int iNoteRow ) const;
	/**
	 * @brief Retrieve the index of the SpeedSegments at the specified beat.
	 * @param fBeat the beat that has a SpeedSegment.
	 * @return the SpeedSegment's index in question.
	 */
	int GetSpeedSegmentIndexAtBeat( float fBeat ) const { return GetSpeedSegmentIndexAtRow( BeatToNoteRow(fBeat) ); }
	/**
	 * @brief Add the SpeedSegment to the TimingData.
	 * @param seg the new SpeedSegment.
	 */
	void AddSpeedSegment( const SpeedSegment &seg );
	
	float GetDisplayedSpeedPercent( float fBeat, float fMusicSeconds ) const;
	
	/**
	 * @brief Retrieve the scrolling factor at the given row.
	 * @param iNoteRow the row in question.
	 * @return the percent.
	 */
	float GetScrollAtRow( int iNoteRow );
	/**
	 * @brief Retrieve the scrolling factor at the given beat.
	 * @param fBeat the beat in question.
	 * @return the percent.
	 */
	float GetScrollAtBeat( float fBeat ) { return GetScrollAtRow( BeatToNoteRow(fBeat) ); }

	/**
	 * @brief Set the row to have the new Scrolling factor.
	 * @param iNoteRow the row to have the new Speed.
	 * @param fPercent the scrolling factor.
	 */
	void SetScrollAtRow( int iNoteRow, float fPercent );
	/**
	 * @brief Set the row to have the new Scrolling factor.
	 * @param iNoteRow the row to have the new Speed.
	 * @param fPercent the scrolling factor.
	 */
	void SetScrollAtBeat( float fBeat, float fPercent ) { SetScrollAtRow( BeatToNoteRow(fBeat), fPercent ); }
	
	/**
	 * @brief Retrieve the ScrollSegment at the specified row.
	 * @param iNoteRow the row that has a ScrollSegment.
	 * @return the ScrollSegment in question.
	 */
	ScrollSegment& GetScrollSegmentAtRow( int iNoteRow );
	/**
	 * @brief Retrieve the ScrollSegment at the specified beat.
	 * @param fBeat the beat that has a ScrollSegment.
	 * @return the ScrollSegment in question.
	 */
	ScrollSegment& GetScrollSegmentAtBeat( float fBeat ) { return GetScrollSegmentAtRow( BeatToNoteRow(fBeat) ); }
	
	/**
	 * @brief Retrieve the index of the ScrollSegment at the specified row.
	 * @param iNoteRow the row that has a ScrollSegment.
	 * @return the ScrollSegment's index in question.
	 */
	int GetScrollSegmentIndexAtRow( int iNoteRow ) const;
	/**
	 * @brief Retrieve the index of the ScrollSegment at the specified beat.
	 * @param fBeat the beat that has a ScrollSegment.
	 * @return the ScrollSegment's index in question.
	 */
	int GetScrollSegmentIndexAtBeat( float fBeat ) const { return GetScrollSegmentIndexAtRow( BeatToNoteRow(fBeat) ); }
	
	/**
	 * @brief Add the ScrollSegment to the TimingData.
	 * @param seg the new ScrollSegment.
	 */
	void AddScrollSegment( const ScrollSegment &seg );
	
	
	/**
	 * @brief Determine when the fakes end.
	 * @param iRow The row you start on.
	 * @return the time when the fakes end.
	 */
	float GetFakeAtRow( int iRow ) const;
	/**
	 * @brief Determine when the fakes end.
	 * @param fBeat The beat you start on.
	 * @return the time when the fakes end.
	 */
	float GetFakeAtBeat( float fBeat ) const { return GetFakeAtRow( BeatToNoteRow( fBeat ) ); }
	/**
	 * @brief Set the beat to indicate when the FakeSegment ends.
	 * @param iRow The row to start on.
	 * @param fNew The destination beat.
	 */
	void SetFakeAtRow( int iRow, float fNew );
	/**
	 * @brief Set the beat to indicate when the FakeSegment ends.
	 * @param fBeat The beat to start on.
	 * @param fNew The destination beat.
	 */
	void SetFakeAtBeat( float fBeat, float fNew ) { SetFakeAtRow( BeatToNoteRow( fBeat ), fNew ); }
	/**
	 * @brief Retrieve the FakeSegment at the specified row.
	 * @param iRow the row to focus on.
	 * @return the FakeSegment in question.
	 */
	FakeSegment& GetFakeSegmentAtRow( int iRow );
	/**
	 * @brief Retrieve the FakeSegment at the specified beat.
	 * @param fBeat the beat to focus on.
	 * @return the FakeSegment in question.
	 */
	FakeSegment& GetFakeSegmentAtBeat( float fBeat ) { return GetFakeSegmentAtRow( BeatToNoteRow( fBeat ) ); }
	/**
	 * @brief Retrieve the index of the FakeSegment at the specified row.
	 * @param iRow the row to focus on.
	 * @return the index in question.
	 */
	int GetFakeSegmentIndexAtRow( int iRow ) const;
	/**
	 * @brief Retrieve the index of the FakeSegment at the specified beat.
	 * @param fBeat the beat to focus on.
	 * @return the index in question.
	 */
	int GetFakeSegmentIndexAtBeat( float fBeat ) const { return GetFakeSegmentIndexAtRow( BeatToNoteRow( fBeat ) ); }
	/**
	 * @brief Checks if the row is inside a fake.
	 * @param iRow the row to focus on.
	 * @return true if the row is inside a fake, false otherwise.
	 */
	bool IsFakeAtRow( int iRow ) const;
	/**
	 * @brief Checks if the beat is inside a fake.
	 * @param fBeat the beat to focus on.
	 * @return true if the row is inside a fake, false otherwise.
	 */
	bool IsFakeAtBeat( float fBeat ) const { return IsFakeAtRow( BeatToNoteRow( fBeat ) ); }
	/**
	 * @brief Add the FakeSegment to the TimingData.
	 * @param seg the new FakeSegment.
	 */
	void AddFakeSegment( const FakeSegment &seg );
	
	/**
	 * @brief Determine if this notes on this row can be judged.
	 * @param row the row to focus on.
	 * @return true if the row can be judged, false otherwise. */
	bool IsJudgableAtRow( int row ) const
	{
		return !(IsWarpAtRow(row) || IsFakeAtRow(row));
	}
	/**
	 * @brief Determine if this notes on this beat can be judged.
	 * @param beat the beat to focus on.
	 * @return true if the row can be judged, false otherwise. */
	bool IsJudgableAtBeat( float beat ) const { return IsJudgableAtRow( BeatToNoteRow( beat ) ); }
	
	
	
	void MultiplyBPMInBeatRange( int iStartIndex, int iEndIndex, float fFactor );
	
	void NoteRowToMeasureAndBeat( int iNoteRow, int &iMeasureIndexOut, int &iBeatIndexOut, int &iRowsRemainder ) const;

	void GetBeatAndBPSFromElapsedTime( float fElapsedTime, float &fBeatOut, float &fBPSOut, bool &bFreezeOut, bool &bDelayOut, int &iWarpBeginOut, float &fWarpLengthOut ) const;
	float GetBeatFromElapsedTime( float fElapsedTime ) const	// shortcut for places that care only about the beat
	{
		float fBeat, fThrowAway, fThrowAway2;
		bool bThrowAway, bThrowAway2;
		int iThrowAway;
		GetBeatAndBPSFromElapsedTime( fElapsedTime, fBeat, fThrowAway, bThrowAway, bThrowAway2, iThrowAway, fThrowAway2 );
		return fBeat;
	}
	float GetElapsedTimeFromBeat( float fBeat ) const;

	void GetBeatAndBPSFromElapsedTimeNoOffset( float fElapsedTime, float &fBeatOut, float &fBPSOut, bool &bFreezeOut, bool &bDelayOut, int &iWarpBeginOut, float &fWarpDestinationOut ) const;
	float GetBeatFromElapsedTimeNoOffset( float fElapsedTime ) const	// shortcut for places that care only about the beat
	{
		float fBeat, fThrowAway, fThrowAway2;
		bool bThrowAway, bThrowAway2;
		int iThrowAway;
		GetBeatAndBPSFromElapsedTimeNoOffset( fElapsedTime, fBeat, fThrowAway, bThrowAway, bThrowAway2, iThrowAway, fThrowAway2 );
		return fBeat;
	}
	float GetElapsedTimeFromBeatNoOffset( float fBeat ) const;
	float GetDisplayedBeat( float fBeat ) const;
	/**
	 * @brief View the TimingData to see if a song changes its BPM at any point.
	 * @return true if there is at least one change, false otherwise.
	 */
	bool HasBpmChanges() const;
	/**
	 * @brief View the TimingData to see if there is at least one stop at any point.
	 *
	 * For the purposes of this function, Stops and Delays are considered the same.
	 * @return true if there is at least one stop, false otherwise.
	 */
	bool HasStops() const;
	/**
	 * @brief View the TimingData to see if there is at least one warp at any point.
	 * @return true if there is at least one warp, false otherwise.
	 */
	bool HasWarps() const;
	/**
	 * @brief View the TimingData to see if there is at least one fake segment involved.
	 * @return true if there is at least one fake segment, false otherwise. */
	bool HasFakes() const;
	/**
	 * @brief View the TimingData to see if a song changes its speed scrolling at any point.
	 * @return true if there is at least one change, false otherwise. */
	bool HasSpeedChanges() const;
	/**
	 * @brief View the TimingData to see if a song changes its speed scrolling at any point.
	 * @return true if there is at least one change, false otherwise. */
	bool HasScrollChanges() const;
	/**
	 * @brief Compare two sets of timing data to see if they are equal.
	 * @param other the other TimingData.
	 * @return the equality or lack thereof of the two TimingData.
	 */
	bool operator==( const TimingData &other )
	{
		COMPARE( m_BPMSegments.size() );
		for( unsigned i=0; i<m_BPMSegments.size(); i++ )
			COMPARE( m_BPMSegments[i] );
		COMPARE( m_StopSegments.size() );
		for( unsigned i=0; i<m_StopSegments.size(); i++ )
			COMPARE( m_StopSegments[i] );
		COMPARE( m_WarpSegments.size() );
		for( unsigned i=0; i<m_WarpSegments.size(); i++ )
			COMPARE( m_WarpSegments[i] );
		COMPARE( m_vTimeSignatureSegments.size() );
		for( unsigned i=0; i<m_vTimeSignatureSegments.size(); i++)
			COMPARE( m_vTimeSignatureSegments[i] );
		COMPARE( m_TickcountSegments.size() );
		for( unsigned i=0; i<m_TickcountSegments.size(); i++ )
			COMPARE( m_TickcountSegments[i] );
		COMPARE( m_ComboSegments.size() );
		for( unsigned i=0; i<m_ComboSegments.size(); i++ )
			COMPARE( m_ComboSegments[i] );
		COMPARE( m_LabelSegments.size() );
		for( unsigned i=0; i<m_LabelSegments.size(); i++ )
			COMPARE( m_LabelSegments[i] );
		COMPARE( m_SpeedSegments.size() );
		for( unsigned i=0; i<m_SpeedSegments.size(); i++ )
			COMPARE( m_SpeedSegments[i] );
		COMPARE( m_ScrollSegments.size() );
		for( unsigned i=0; i<m_ScrollSegments.size(); i++ )
			COMPARE( m_ScrollSegments[i] );
		COMPARE( m_FakeSegments.size() );
		for( unsigned i=0; i<m_FakeSegments.size(); i++ )
			COMPARE( m_FakeSegments[i] );
		COMPARE( m_fBeat0OffsetInSeconds );
		return true;
	}
	/**
	 * @brief Compare two sets of timing data to see if they are not equal.
	 * @param other the other TimingData.
	 * @return the inequality or lack thereof of the two TimingData.
	 */
	bool operator!=( const TimingData &other ) { return !operator==(other); }

	void ScaleRegion( float fScale = 1, int iStartRow = 0, int iEndRow = MAX_NOTE_ROW, bool bAdjustBPM = false );
	void InsertRows( int iStartRow, int iRowsToAdd );
	void DeleteRows( int iStartRow, int iRowsToDelete );

	/**
	 * @brief Tidy up the timing data, e.g. provide default BPMs, labels, tickcounts.
	 */
	void TidyUpData();

	// Lua
	void PushSelf( lua_State *L );
	/**
	 * @brief The file of the song/steps that use this TimingData.
	 *
	 * This is for informational purposes only.
	 */
	RString					m_sFile;
	// All of the following vectors must be sorted before gameplay.
	/**
	 * @brief The collection of BPMSegments.
	 */
	vector<BPMSegment>		m_BPMSegments;
	/**
	 * @brief The collection of StopSegments & DelaySegments.
	 */
	vector<StopSegment>		m_StopSegments;
	/**
	 * @brief The collection of TimeSignatureSegments.
	 */
	vector<TimeSignatureSegment>	m_vTimeSignatureSegments;
	/**
	 * @brief The collection of WarpSegments.
	 */
	vector<WarpSegment>		m_WarpSegments;
	/**
	 * @brief The collection of TickcountSegments.
	 */
	vector<TickcountSegment>	m_TickcountSegments;
	/**
	 * @brief The collection of ComboSegments.
	 */
	vector<ComboSegment>		m_ComboSegments;
	/**
	 * @brief The collection of LabelSegments.
	 */
	vector<LabelSegment>		m_LabelSegments;
	/** @brief The collection of SpeedSegments. */
	vector<SpeedSegment>		m_SpeedSegments;
	/** @brief The collection of ScrollSegments. */
	vector<ScrollSegment>		m_ScrollSegments;
	/** @brief The collection of FakeSegments. */
	vector<FakeSegment>		m_FakeSegments;
	/**
	 * @brief The initial offset of a song.
	 */
	float	m_fBeat0OffsetInSeconds;
	/**
	 * @brief A flag to see if negative BPMs are used in this song.
	 *
	 * This is only used for Lua bindings in the Song, with the intentions that
	 * this is moved into TimingData at a future point in time.
	 */
	bool	m_bHasNegativeBpms;
};

#undef COMPARE

#endif

/**
 * @file
 * @author Chris Danford, Glenn Maynard (c) 2001-2004 
 * @section LICENSE
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
