
#if defined _inc_timestamp_
	#endinput
#endif
#define _inc_timestamp_

#define EPOCH_YEAR 1970

#define MAX_TIMESTAMP_FORMAT_LENGTH 128

enum TIME_UNIT 
{
    UNIT_YEAR,
    UNIT_MONTH,
    UNIT_DAY,
    UNIT_HOUR,
    UNIT_MINUTE
};

static MonthTimes[12][4] = 
{
    { 31, 31, 2678400, 2678400 },
    { 28, 29, 2419200, 2505600 },
    { 31, 31, 2678400, 2678400 },
    { 30, 30, 2592000, 2592000 },
    { 31, 31, 2678400, 2678400 },
    { 30, 30, 2592000, 2592000 },
    { 31, 31, 2678400, 2678400 },
    { 31, 31, 2678400, 2678400 },
    { 30, 30, 2592000, 2592000 },
    { 31, 31, 2678400, 2678400 },
    { 30, 30, 2592000, 2592000 },
    { 31, 31, 2678400, 2678400 }
};

static const MONTH_DAYS[12]	= {
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

static const MONTH_NAMES[12][] = {
	"Thang 1", "Thang 2", "Thang 3", "Thang 4", "Thang 5", "Thang 6", "Thang 7", "Thang 8", "Thang 9", "Thang 10", "Thang 11", "Thang 12"
};

stock bool:IsLeapYear(year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

stock GetMonthDays(month, year) {
	if (month == 1 && IsLeapYear(year) == true) {
	    return 29;
	}

	return MONTH_DAYS[month - 1];
}

stock GetDayOfWeek(year, month, day) {
    if (month < 3) {
        month += 12;
        year -= 1;
    }
    new K = year / 100;
    new J = year % 100;
    new f = day + ((13 * (month + 1)) / 5) + J + (J / 4) + (K / 4) - (2 * K);
    return ((f + 5) % 7) + 1; 
}


stock bool:IsTimeInRange(hour, minute, startHour, startMinute, endHour, endMinute)
{
    if (startHour < endHour || (startHour == endHour && startMinute <= endMinute))
    {
        if ((hour > startHour || (hour == startHour && minute >= startMinute)) &&
            (hour < endHour || (hour == endHour && minute <= endMinute)))
        {
            return true;
        }
    }
    else
    {
        if ((hour > startHour || (hour == startHour && minute >= startMinute)) ||
            (hour < endHour || (hour == endHour && minute <= endMinute)))
        {
            return true;
        }
    }
    return false;
}

static stock ReplaceString(string[], start, end, const replacement[], size) {
	strdel(string, start, end);
	strins(string, replacement, start, size);
}

stock mktime(year, month, day, hour, minute, second)
{
    static const
		SECONDS_PER_MINUTE = 60,
		SECONDS_PER_HOUR = 3600,
		SECONDS_PER_DAY = 86400;

    new totalDays = 0;
    new totalSeconds = 0;
    new daysInMonth;
    new i;

    for (i = 1970; i < year; i++)
    {
        totalDays += IsLeapYear(i) ? 366 : 365;
    }

    for (i = 1; i < month; i++)
    {
        daysInMonth = GetMonthDays(i, year);
        totalDays += daysInMonth;
    }

    totalDays += day - 1;

    totalSeconds = totalDays * SECONDS_PER_DAY + hour * SECONDS_PER_HOUR + minute * SECONDS_PER_MINUTE + second;

	// error for GMT +7 time zone
	totalSeconds += 25200 + 208800;

    return totalSeconds;
}

stock Timestamp(year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0, hourGMT = 0, minuteGMT = 0) {
    static const
		SECONDS_PER_MINUTE = 60,
		SECONDS_PER_HOUR = 3600,
		SECONDS_PER_DAY = 86400;

	new timestamp = 0;

	if (year == 0 || month == 0 || day == 0) {
	    getdate(year, month, day);
	    gettime(hour, minute, second);
	}

	for (new i = EPOCH_YEAR; i < year; i++)
	    day += (IsLeapYear(i) == true) ? 365 : 366;

	for (new i = 1; i < month; i++)
		day += MONTH_DAYS[i - 1];

	if (IsLeapYear(year) == true && month > 2)
	    day += 1;

    timestamp += ((day - 1) * SECONDS_PER_DAY);
    timestamp += ((hour + hourGMT) * SECONDS_PER_HOUR);
    timestamp += ((minute + minuteGMT) * SECONDS_PER_MINUTE);
    timestamp += second;

	return timestamp;
}

stock ParseTimestamp(Timestamp, &year, &month, &day, &hour, &minute, &second, HourGMT, MinuteGMT = 0)
{
    new tmp = 2;
    year = 1970;
    month = 1;
    Timestamp -= 172800; // Delete two days from the current timestamp. This is necessary, because the timestamp retrieved using gettime() includes two too many days.
    for(;;)
    {
        if(Timestamp >= 31536000)
        {
            year ++;
            Timestamp -= 31536000;
            tmp ++;
            if(tmp == 4)
            {
                if(Timestamp >= 31622400)
                {
                    tmp = 0;
                    year ++;
                    Timestamp -= 31622400;
                }
                else break;
            }
        }
        else break;
    }       
    for(new i = 0, leap; i < 12; i ++)
    {
		leap = IsLeapYear(year) ? 1 : 0;
        if(Timestamp >= MonthTimes[i][2 + leap])
        {
            month ++;
            Timestamp -= MonthTimes[i][2 + leap];
        }
        else break;
    }
    day = 1 + (Timestamp / 86400);
    Timestamp %= 86400;
    hour = HourGMT + (Timestamp / 3600);
    Timestamp %= 3600;
    minute = MinuteGMT + (Timestamp / 60);
    second = (Timestamp % 60);
    if(minute > 59)
    {
        minute = 0;
        hour ++;
    }
    if(hour > 23)
    {
        hour -= 24;
        day ++;
    }   
    if(day > MonthTimes[month][IsLeapYear(year)])
    {
        day = 1;
        month ++;
    }
    if(month > 12)
    {
        month = 1;
        year ++;
    }
    return 1;
}

stock FormatTimestamp(timestamp, const format[], hourGMT = 0, minuteGMT = 0) {
	new string[MAX_TIMESTAMP_FORMAT_LENGTH];
	strcat(string, format);
	
	new year, month, day, hour, minute, second;
	ParseTimestamp(timestamp, year, month, day, hour, minute, second, hourGMT, minuteGMT);

	new pos = -1;

	do {
	    // year abbreviated (last 2 chars)
		if ((pos = strfind(string, "%y")) != -1) {
			new replacement[4];
			valstr(replacement, year % 100);
			ReplaceString(string, pos, pos + 2, replacement, MAX_TIMESTAMP_FORMAT_LENGTH);
		}
		// full year
		else if ((pos = strfind(string, "%Y")) != -1) {
			new replacement[5];
			valstr(replacement, year);
			ReplaceString(string, pos, pos + 2, replacement, MAX_TIMESTAMP_FORMAT_LENGTH);
		}
		// month number
		else if ((pos = strfind(string, "%m")) != -1) {
			new replacement[4];
			valstr(replacement, month);
			ReplaceString(string, pos, pos + 2, replacement, MAX_TIMESTAMP_FORMAT_LENGTH);
		}
		// month name abbreviated
		else if ((pos = strfind(string, "%b")) != -1) {
		    new replacement[4];
		    strcat(replacement, MONTH_NAMES[month - 1]);
			ReplaceString(string, pos, pos + 2, replacement, MAX_TIMESTAMP_FORMAT_LENGTH);
		}
		// full month name
		else if ((pos = strfind(string, "%B")) != -1) {
			ReplaceString(string, pos, pos + 2, MONTH_NAMES[month - 1], MAX_TIMESTAMP_FORMAT_LENGTH);
		}
		// day
		else if ((pos = strfind(string, "%d")) != -1) {
			new replacement[4];
			valstr(replacement, day);
			ReplaceString(string, pos, pos + 2, replacement, MAX_TIMESTAMP_FORMAT_LENGTH);
		}
		// hour 24H
		else if ((pos = strfind(string, "%H")) != -1) {
			new replacement[4];
			valstr(replacement, hour);
			ReplaceString(string, pos, pos + 2, replacement, MAX_TIMESTAMP_FORMAT_LENGTH);
		}
		// hour 12H
		else if ((pos = strfind(string, "%I")) != -1) {
			new replacement[4];
			valstr(replacement, (hour > 12) ? (hour - 12) : (hour));
			ReplaceString(string, pos, pos + 2, replacement, MAX_TIMESTAMP_FORMAT_LENGTH);
		}
		// hour 12H AM or PM
		else if ((pos = strfind(string, "%p")) != -1) {
			ReplaceString(string, pos, pos + 2, (hour > 12) ? ("PM") : ("AM"), MAX_TIMESTAMP_FORMAT_LENGTH);
		}
		// minute
		else if ((pos = strfind(string, "%M")) != -1) {
			new replacement[4];
			valstr(replacement, minute);
			ReplaceString(string, pos, pos + 2, replacement, MAX_TIMESTAMP_FORMAT_LENGTH);
		}
		// second
		else if ((pos = strfind(string, "%S")) != -1) {
			new replacement[4];
			valstr(replacement, second);
			ReplaceString(string, pos, pos + 2, replacement, MAX_TIMESTAMP_FORMAT_LENGTH);
		}
	} while (pos != -1);
	
	return string;
}

stock GetRemainingTime(startTimestamp, endTimestamp) 
{
    new timeDiff = endTimestamp - startTimestamp;

	new timeString[9];
	strcat(timeString, "00:00:00");

    if (timeDiff < 0) {
        return timeString;
    }

    new fhours = timeDiff / 3600;
    new fminutes = (timeDiff % 3600) / 60;
    new fseconds = timeDiff % 60;

    format(timeString, sizeof(timeString), "%02d:%02d:%02d", fhours, fminutes, fseconds);
    return timeString;
}

stock FormatTimeleft(startTimestamp, endTimestamp) {
    static const
		SECONDS_PER_MINUTE = 60,
		SECONDS_PER_HOUR = 3600,
		SECONDS_PER_DAY = 86400,
		SECONDS_PER_MONTH = 2592000;

    new string[MAX_TIMESTAMP_FORMAT_LENGTH];

	new seconds = endTimestamp - startTimestamp;

	if (seconds == 1)
		format(string, sizeof(string), "mot giay");
	else if (seconds < SECONDS_PER_MINUTE)
		format(string, sizeof(string), "%i giay", seconds);
	else if (seconds < (2 * SECONDS_PER_MINUTE))
		format(string, sizeof(string), "mot phut");
	else if (seconds < (45 * SECONDS_PER_MINUTE))
		format(string, sizeof(string), "%i phut", (seconds / SECONDS_PER_MINUTE));
	else if (seconds < (90 * SECONDS_PER_MINUTE))
		format(string, sizeof(string), "mot gio");
	else if (seconds < (24 * SECONDS_PER_HOUR))
		format(string, sizeof(string), "%i gio", (seconds / SECONDS_PER_HOUR));
	else if (seconds < (48 * SECONDS_PER_HOUR))
		format(string, sizeof(string), "mot ngay");
	else if (seconds < (30 * SECONDS_PER_DAY))
		format(string, sizeof(string), "%i ngay", (seconds / SECONDS_PER_DAY));
	else if (seconds < (12 * SECONDS_PER_MONTH)) {
		new months = floatround(seconds / SECONDS_PER_DAY / 30);
      	if (months <= 1)
			format(string, sizeof(string), "mot thang");
      	else
			format(string, sizeof(string), "%i thang", months);
	}
    else {
      	new years = floatround(seconds / SECONDS_PER_DAY / 365);
      	if (years <= 1)
			format(string, sizeof(string), "mot nam");
      	else
			format(string, sizeof(string), "%i nam", years);
	}

	return string;
}

stock ConvertFromSeconds(TIME_UNIT:type, seconds) {
    static const
		SECONDS_PER_MINUTE = 60,
		SECONDS_PER_HOUR = 3600,
		SECONDS_PER_DAY = 86400,
		SECONDS_PER_YEAR = 31556952;

	switch (type) {
		case UNIT_YEAR:
			return seconds / SECONDS_PER_YEAR;
		case UNIT_MONTH:
		    return seconds / (SECONDS_PER_DAY * 31);
		case UNIT_DAY:
		    return seconds / SECONDS_PER_DAY;
		case UNIT_HOUR:
		    return seconds / SECONDS_PER_HOUR;
		case UNIT_MINUTE:
		    return seconds / SECONDS_PER_MINUTE;
	}

	return 0;
}

stock ConvertToSeconds(TIME_UNIT:type, value) {
    static const
		SECONDS_PER_MINUTE = 60,
		SECONDS_PER_HOUR = 3600,
		SECONDS_PER_DAY = 86400,
		SECONDS_PER_YEAR = 31556952;

	switch (type) {
		case UNIT_YEAR:
			return SECONDS_PER_YEAR * value;
		case UNIT_MONTH:
		    return SECONDS_PER_DAY * (31 * value);
		case UNIT_DAY:
		    return SECONDS_PER_DAY * value;
		case UNIT_HOUR:
		    return SECONDS_PER_HOUR * value;
		case UNIT_MINUTE:
		    return SECONDS_PER_MINUTE * value;
	}

	return 0;
}