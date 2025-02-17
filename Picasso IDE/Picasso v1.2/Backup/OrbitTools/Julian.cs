//
// Julian.cs
//
// This class encapsulates Julian dates with the epoch of 12:00 noon (12:00 UT)
// on January 1, 4713 B.C. Some epoch dates:
//    01/01/1990 00:00 UTC - 2447892.5
//    01/01/1990 12:00 UTC - 2447893.0
//    01/01/2000 00:00 UTC - 2451544.5
//    01/01/2001 00:00 UTC - 2451910.5
//
// Note the Julian day begins at noon, which allows astronomers to have the
// same date in a single observing session.
//
// References:
// "Astronomical Formulae for Calculators", Jean Meeus
// "Satellite Communications", Dennis Roddy, 2nd Edition, 1995.
// "Spacecraft Attitude Determination and Contol", James R. Wertz, 1984
//
// Copyright (c) 2003-2009 Michael F. Henry
//
using System;

namespace Zeptomoby.OrbitTools
{
	/// <summary>
	/// Encapsulates a Julian date.
	/// </summary>
   public class Julian
   {
      private const double EPOCH_JAN1_00H_1900 = 2415020.5; // Jan  1.0 1900 = Jan  1 1900 00h UTC
      private const double EPOCH_JAN1_12H_1900 = 2415021.0; // Jan  1.5 1900 = Jan  1 1900 12h UTC
      private const double EPOCH_JAN0_12H_1900 = 2415020.0; // Dec 31.5 1899 = Dec 31 1899 12h UTC
      private const double EPOCH_JAN1_12H_2000 = 2451545.0; // Jan  1.5 2000 = Jan  1 2000 12h UTC

      private double m_Date; // Julian date
      private int    m_Year; // Year including century
      private double m_Day;  // Day of year, 1.0 = Jan 1 00h

      #region Construction

      // /////////////////////////////////////////////////////////////////////
      // Create a Julian date object from a DateTime object. The time
      // contained in the DateTime object is assumed to be UTC.
      public Julian(DateTime dt)
      {
         double day = dt.DayOfYear + 
            (dt.Hour + 
            ((dt.Minute + 
            ((dt.Second + (dt.Millisecond / 1000.0)) / 60.0)) / 60.0)) / 24.0;

         Initialize(dt.Year, day);
      }

      // /////////////////////////////////////////////////////////////////////
      // Create a Julian date object from a year and day of year.
      // The year is given with the century (i.e. 2001).
      // The integer part of the day value is the day of year, with 1 meaning
      // January 1.
      // The fractional part of the day value is the fractional portion of
      // the day.
      // Examples: 
      //    day = 1.0  Jan 1 00h
      //    day = 1.5  Jan 1 12h
      //    day = 2.0  Jan 2 00h
      public Julian(int year, double day)
      {
         Initialize(year, day);
      }

      #endregion

      #region Properties

      public double Date { get { return m_Date; } }

      public double FromJan1_00h_1900() { return m_Date - EPOCH_JAN1_00H_1900; }
      public double FromJan1_12h_1900() { return m_Date - EPOCH_JAN1_12H_1900; }
      public double FromJan0_12h_1900() { return m_Date - EPOCH_JAN0_12H_1900; }
      public double FromJan1_12h_2000() { return m_Date - EPOCH_JAN1_12H_2000; }

      #endregion

      // /////////////////////////////////////////////////////////////////////
      public TimeSpan Diff(Julian date)
      {
         const double TICKS_PER_DAY = 8.64e11; // 1 tick = 100 nanoseconds
         return new TimeSpan((long)((m_Date - date.m_Date) * TICKS_PER_DAY));
      }

      // /////////////////////////////////////////////////////////////////////
      // Initialize the Julian object.
      // The first day of the year, Jan 1, is day 1.0. Noon on Jan 1 is
      // represented by the day value of 1.5, etc.
      protected void Initialize(int year, double day)
      {
         // Arbitrary years used for error checking
         if (year < 1900 || year > 2100)
         {
            throw new ArgumentOutOfRangeException("year");
         }

         // The last day of a leap year is day 366
         if (day < 1.0 || day >= 367.0)
         {
            throw new ArgumentOutOfRangeException("day");
         }

         m_Year = year;
         m_Day  = day;

         // Now calculate Julian date
         // Ref: "Astronomical Formulae for Calculators", Jean Meeus, pages 23-25

         year--;

         // Centuries are not leap years unless they divide by 400
         int A = (year / 100);
         int B = 2 - A + (A / 4);

         double NewYears = (int)(365.25 * year) +
                           (int)(30.6001 * 14)  + 
                           1720994.5 + B;

         m_Date = NewYears + day;
      }

      // /////////////////////////////////////////////////////////////////////
      // toGMST()
      // Calculate Greenwich Mean Sidereal Time for the Julian date. The 
      // return value is the angle, in radians, measuring eastward from the
      // Vernal Equinox to the prime meridian. This angle is also referred
      // to as "ThetaG" (Theta GMST).
      // 
      // References:
      //    The 1992 Astronomical Almanac, page B6.
      //    Explanatory Supplement to the Astronomical Almanac, page 50.
      //    Orbital Coordinate Systems, Part III, Dr. T.S. Kelso, 
      //       Satellite Times, Nov/Dec 1995
      public double toGMST() 
      {
         double UT = (m_Date + 0.5) % 1.0;
         double TU = (FromJan1_12h_2000() - UT) / 36525.0;

         double GMST = 24110.54841 + TU * 
                       (8640184.812866 + TU * (0.093104 - TU * 6.2e-06));

         GMST = (GMST + Globals.SEC_PER_DAY * Globals.OMEGA_E * UT) % Globals.SEC_PER_DAY;
   
         if (GMST < 0.0)
            GMST += Globals.SEC_PER_DAY;  // "wrap" negative modulo value

         return  (Globals.TWOPI * (GMST / Globals.SEC_PER_DAY));
      }

      // /////////////////////////////////////////////////////////////////////
      // toLMST()
      // Calculate Local Mean Sidereal Time for given longitude (for this date).
      // The longitude is assumed to be in radians measured west from Greenwich.
      // The return value is the angle, in radians, measuring eastward from the
      // Vernal Equinox to the given longitude.
      public double toLMST(double lon)
      {
         return (toGMST() + lon) % Globals.TWOPI;
      }

      // /////////////////////////////////////////////////////////////////////
      // toTime()
      // Convert to type DateTime.
      public DateTime toTime()
      {
         // Jan 1
         DateTime dt = new DateTime(m_Year, 1, 1);

         // m_Day = 1 = Jan1
         dt = dt.AddDays(m_Day - 1.0);

         return dt;
      }
   }
}
