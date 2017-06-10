/* Evolution calendar utilities and types
 *
 * Copyright (C) 1999-2008 Novell, Inc. (www.novell.com)
 *
 * This library is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors: Federico Mena-Quintero <federico@ximian.com>
 */

#include <config.h>
#include <stdlib.h>
#include <string.h>
#include <glib/gstdio.h>
#include <glib/gi18n-lib.h>

#include <libedataserver/libedataserver.h>

#include "e-cal-util.h"
#include "e-cal-system-timezone.h"

#define _TIME_MIN	((time_t) 0)		/* Min valid time_t	*/
#define _TIME_MAX	((time_t) INT_MAX)

/**
 * cal_obj_instance_list_free:
 * @list: List of #CalObjInstance structures.
 *
 * Frees a list of #CalObjInstance structures.
 **/
void
cal_obj_instance_list_free (GList *list)
{
	CalObjInstance *i;
	GList *l;

	for (l = list; l; l = l->next) {
		i = l->data;

		if (i != NULL && i->uid != NULL) {
			g_free (i->uid);
			g_free (i);
		} else
			g_warn_if_reached ();
	}

	g_list_free (list);
}

/**
 * cal_obj_uid_list_free:
 * @list: List of strings with unique identifiers.
 *
 * Frees a list of unique identifiers for calendar objects.
 **/
void
cal_obj_uid_list_free (GList *list)
{
	g_list_foreach (list, (GFunc) g_free, NULL);
	g_list_free (list);
}

/**
 * e_cal_util_new_top_level:
 *
 * Creates a new VCALENDAR component.
 *
 * Returns: the newly created top level component.
 */
icalcomponent *
e_cal_util_new_top_level (void)
{
	icalcomponent *icalcomp;
	icalproperty *prop;

	icalcomp = icalcomponent_new (ICAL_VCALENDAR_COMPONENT);

	/* RFC 2445, section 4.7.1 */
	prop = icalproperty_new_calscale ("GREGORIAN");
	icalcomponent_add_property (icalcomp, prop);

       /* RFC 2445, section 4.7.3 */
	prop = icalproperty_new_prodid ("-//Ximian//NONSGML Evolution Calendar//EN");
	icalcomponent_add_property (icalcomp, prop);

	/* RFC 2445, section 4.7.4.  This is the iCalendar spec version, *NOT*
	 * the product version!  Do not change this!
	 */
	prop = icalproperty_new_version ("2.0");
	icalcomponent_add_property (icalcomp, prop);

	return icalcomp;
}

/**
 * e_cal_util_new_component:
 * @kind: Kind of the component to create.
 *
 * Creates a new #icalcomponent of the specified kind.
 *
 * Returns: the newly created component.
 */
icalcomponent *
e_cal_util_new_component (icalcomponent_kind kind)
{
	icalcomponent *comp;
	struct icaltimetype dtstamp;
	gchar *uid;

	comp = icalcomponent_new (kind);
	uid = e_cal_component_gen_uid ();
	icalcomponent_set_uid (comp, uid);
	g_free (uid);
	dtstamp = icaltime_current_time_with_zone (icaltimezone_get_utc_timezone ());
	icalcomponent_set_dtstamp (comp, dtstamp);

	return comp;
}

static gchar *
read_line (const gchar *string)
{
	GString *line_str = NULL;

	for (; *string; string++) {
		if (!line_str)
			line_str = g_string_new ("");

		line_str = g_string_append_c (line_str, *string);
		if (*string == '\n')
			break;
	}

	return g_string_free (line_str, FALSE);
}

/**
 * e_cal_util_parse_ics_string:
 * @string: iCalendar string to be parsed.
 *
 * Parses an iCalendar string and returns a new #icalcomponent representing
 * that string. Note that this function deals with multiple VCALENDAR's in the
 * string, something that Mozilla used to do and which libical does not
 * support.
 *
 * Returns: a newly created #icalcomponent or NULL if the string isn't a
 * valid iCalendar string.
 */
icalcomponent *
e_cal_util_parse_ics_string (const gchar *string)
{
	GString *comp_str = NULL;
	gchar *s;
	icalcomponent *icalcomp = NULL;

	g_return_val_if_fail (string != NULL, NULL);

	/* Split string into separated VCALENDAR's, if more than one */
	s = g_strstr_len (string, strlen (string), "BEGIN:VCALENDAR");

	if (s == NULL)
		return icalparser_parse_string (string);

	while (*s != '\0') {
		gchar *line = read_line (s);

		if (!comp_str)
			comp_str = g_string_new (line);
		else
			comp_str = g_string_append (comp_str, line);

		if (strncmp (line, "END:VCALENDAR", 13) == 0) {
			icalcomponent *tmp;

			tmp = icalparser_parse_string (comp_str->str);
			if (tmp && icalcomponent_isa (tmp) == ICAL_VCALENDAR_COMPONENT) {
				if (icalcomp)
					icalcomponent_merge_component (icalcomp, tmp);
				else
					icalcomp = tmp;
			} else {
				g_warning (
					"Could not merge the components, "
					"the component is either invalid "
					"or not a toplevel component \n");
			}

			g_string_free (comp_str, TRUE);
			comp_str = NULL;
		}

		s += strlen (line);

		g_free (line);
	}

	return icalcomp;
}

static gchar *
get_line_fn (gchar *buf,
             gsize size,
             gpointer file)
{
	return fgets (buf, size, file);
}

/**
 * e_cal_util_parse_ics_file:
 * @filename: Name of the file to be parsed.
 *
 * Parses the given file, and, if it contains a valid iCalendar object,
 * parse it and return a new #icalcomponent.
 *
 * Returns: a newly created #icalcomponent or NULL if the file doesn't
 * contain a valid iCalendar object.
 */
icalcomponent *
e_cal_util_parse_ics_file (const gchar *filename)
{
	icalparser *parser;
	icalcomponent *icalcomp;
	FILE *file;

	file = g_fopen (filename, "rb");
	if (!file)
		return NULL;

	parser = icalparser_new ();
	icalparser_set_gen_data (parser, file);

	icalcomp = icalparser_parse (parser, get_line_fn);
	icalparser_free (parser);
	fclose (file);

	return icalcomp;
}

/* Computes the range of time in which recurrences should be generated for a
 * component in order to compute alarm trigger times.
 */
static void
compute_alarm_range (ECalComponent *comp,
                     GList *alarm_uids,
                     time_t start,
                     time_t end,
                     time_t *alarm_start,
                     time_t *alarm_end)
{
	GList *l;
	time_t repeat_time;

	*alarm_start = start;
	*alarm_end = end;

	repeat_time = 0;

	for (l = alarm_uids; l; l = l->next) {
		const gchar *auid;
		ECalComponentAlarm *alarm;
		ECalComponentAlarmTrigger trigger;
		struct icaldurationtype *dur;
		time_t dur_time;
		ECalComponentAlarmRepeat repeat;

		auid = l->data;
		alarm = e_cal_component_get_alarm (comp, auid);
		g_return_if_fail (alarm != NULL);

		e_cal_component_alarm_get_trigger (alarm, &trigger);
		e_cal_component_alarm_get_repeat (alarm, &repeat);
		e_cal_component_alarm_free (alarm);

		switch (trigger.type) {
		case E_CAL_COMPONENT_ALARM_TRIGGER_NONE:
		case E_CAL_COMPONENT_ALARM_TRIGGER_ABSOLUTE:
			break;

		case E_CAL_COMPONENT_ALARM_TRIGGER_RELATIVE_START:
		case E_CAL_COMPONENT_ALARM_TRIGGER_RELATIVE_END:
			dur = &trigger.u.rel_duration;
			dur_time = icaldurationtype_as_int (*dur);

			if (repeat.repetitions != 0) {
				gint rdur;

				rdur = repeat.repetitions *
					icaldurationtype_as_int (repeat.duration);
				repeat_time = MAX (repeat_time, rdur);
			}

			if (dur->is_neg)
				/* If the duration is negative then dur_time
				 * will be negative as well; that is why we
				 * subtract to expand the range.
				 */
				*alarm_end = MAX (*alarm_end, end - dur_time);
			else
				*alarm_start = MIN (*alarm_start, start - dur_time);

			break;

		default:
			g_return_if_reached ();
		}
	}

	*alarm_start -= repeat_time;
	g_warn_if_fail (*alarm_start <= *alarm_end);
}

/* Closure data to generate alarm occurrences */
struct alarm_occurrence_data {
	/* These are the info we have */
	GList *alarm_uids;
	time_t start;
	time_t end;
	ECalComponentAlarmAction *omit;

	/* This is what we compute */
	GSList *triggers;
	gint n_triggers;
};

static void
add_trigger (struct alarm_occurrence_data *aod,
             const gchar *auid,
             time_t trigger,
             time_t occur_start,
             time_t occur_end)
{
	ECalComponentAlarmInstance *instance;

	instance = g_new (ECalComponentAlarmInstance, 1);
	instance->auid = g_strdup (auid);
	instance->trigger = trigger;
	instance->occur_start = occur_start;
	instance->occur_end = occur_end;

	aod->triggers = g_slist_prepend (aod->triggers, instance);
	aod->n_triggers++;
}

/* Callback used from cal_recur_generate_instances(); generates triggers for all
 * of a component's RELATIVE alarms.
 */
static gboolean
add_alarm_occurrences_cb (ECalComponent *comp,
                          time_t start,
                          time_t end,
                          gpointer data)
{
	struct alarm_occurrence_data *aod;
	GList *l;

	aod = data;

	for (l = aod->alarm_uids; l; l = l->next) {
		const gchar *auid;
		ECalComponentAlarm *alarm;
		ECalComponentAlarmAction action;
		ECalComponentAlarmTrigger trigger;
		ECalComponentAlarmRepeat repeat;
		struct icaldurationtype *dur;
		time_t dur_time;
		time_t occur_time, trigger_time;
		gint i;

		auid = l->data;
		alarm = e_cal_component_get_alarm (comp, auid);
		g_return_val_if_fail (alarm != NULL, FALSE);

		e_cal_component_alarm_get_action (alarm, &action);
		e_cal_component_alarm_get_trigger (alarm, &trigger);
		e_cal_component_alarm_get_repeat (alarm, &repeat);
		e_cal_component_alarm_free (alarm);

		for (i = 0; aod->omit[i] != -1; i++) {
			if (aod->omit[i] == action)
				break;
		}
		if (aod->omit[i] != -1)
			continue;

		if (trigger.type != E_CAL_COMPONENT_ALARM_TRIGGER_RELATIVE_START
		    && trigger.type != E_CAL_COMPONENT_ALARM_TRIGGER_RELATIVE_END)
			continue;

		dur = &trigger.u.rel_duration;
		dur_time = icaldurationtype_as_int (*dur);

		if (trigger.type == E_CAL_COMPONENT_ALARM_TRIGGER_RELATIVE_START)
			occur_time = start;
		else
			occur_time = end;

		/* If dur->is_neg is true then dur_time will already be
		 * negative.  So we do not need to test for dur->is_neg here; we
		 * can simply add the dur_time value to the occur_time and get
		 * the correct result.
		 */

		trigger_time = occur_time + dur_time;

		/* Add repeating alarms */

		if (repeat.repetitions != 0) {
			gint i;
			time_t repeat_time;

			repeat_time = icaldurationtype_as_int (repeat.duration);

			for (i = 0; i < repeat.repetitions; i++) {
				time_t t;

				t = trigger_time + (i + 1) * repeat_time;

				if (t >= aod->start && t < aod->end)
					add_trigger (aod, auid, t, start, end);
			}
		}

		/* Add the trigger itself */

		if (trigger_time >= aod->start && trigger_time < aod->end)
			add_trigger (aod, auid, trigger_time, start, end);
	}

	return TRUE;
}

/* Generates the absolute triggers for a component */
static void
generate_absolute_triggers (ECalComponent *comp,
                            struct alarm_occurrence_data *aod,
                            ECalRecurResolveTimezoneFn resolve_tzid,
                            gpointer user_data,
                            icaltimezone *default_timezone)
{
	GList *l;
	ECalComponentDateTime dt_start, dt_end;

	e_cal_component_get_dtstart (comp, &dt_start);
	e_cal_component_get_dtend (comp, &dt_end);

	for (l = aod->alarm_uids; l; l = l->next) {
		const gchar *auid;
		ECalComponentAlarm *alarm;
		ECalComponentAlarmAction action;
		ECalComponentAlarmRepeat repeat;
		ECalComponentAlarmTrigger trigger;
		time_t abs_time;
		time_t occur_start, occur_end;
		icaltimezone *zone;
		gint i;

		auid = l->data;
		alarm = e_cal_component_get_alarm (comp, auid);
		g_return_if_fail (alarm != NULL);

		e_cal_component_alarm_get_action (alarm, &action);
		e_cal_component_alarm_get_trigger (alarm, &trigger);
		e_cal_component_alarm_get_repeat (alarm, &repeat);
		e_cal_component_alarm_free (alarm);

		for (i = 0; aod->omit[i] != -1; i++) {
			if (aod->omit[i] == action)
				break;
		}
		if (aod->omit[i] != -1)
			continue;

		if (trigger.type != E_CAL_COMPONENT_ALARM_TRIGGER_ABSOLUTE)
			continue;

		/* Absolute triggers are always in UTC;
		 * see RFC 2445 section 4.8.6.3 */
		zone = icaltimezone_get_utc_timezone ();

		abs_time = icaltime_as_timet_with_zone (trigger.u.abs_time, zone);

		/* No particular occurrence, so just use the times from the
		 * component */

		if (dt_start.value) {
			if (dt_start.tzid && !dt_start.value->is_date)
				zone = (* resolve_tzid) (dt_start.tzid, user_data);
			else
				zone = default_timezone;

			occur_start = icaltime_as_timet_with_zone (
				*dt_start.value, zone);
		} else
			occur_start = -1;

		if (dt_end.value) {
			if (dt_end.tzid && !dt_end.value->is_date)
				zone = (* resolve_tzid) (dt_end.tzid, user_data);
			else
				zone = default_timezone;

			occur_end = icaltime_as_timet_with_zone (*dt_end.value, zone);
		} else
			occur_end = -1;

		/* Add repeating alarms */

		if (repeat.repetitions != 0) {
			gint i;
			time_t repeat_time;

			repeat_time = icaldurationtype_as_int (repeat.duration);

			for (i = 0; i < repeat.repetitions; i++) {
				time_t t;

				t = abs_time + (i + 1) * repeat_time;

				if (t >= aod->start && t < aod->end)
					add_trigger (
						aod, auid, t,
						occur_start, occur_end);
			}
		}

		/* Add the trigger itself */

		if (abs_time >= aod->start && abs_time < aod->end)
			add_trigger (aod, auid, abs_time, occur_start, occur_end);
	}

	e_cal_component_free_datetime (&dt_start);
	e_cal_component_free_datetime (&dt_end);
}

/* Compares two alarm instances; called from g_slist_sort() */
static gint
compare_alarm_instance (gconstpointer a,
                        gconstpointer b)
{
	const ECalComponentAlarmInstance *aia, *aib;

	aia = a;
	aib = b;

	if (aia->trigger < aib->trigger)
		return -1;
	else if (aia->trigger > aib->trigger)
		return 1;
	else
		return 0;
}

/**
 * e_cal_util_generate_alarms_for_comp:
 * @comp: The #ECalComponent to generate alarms from
 * @start: Start time
 * @end: End time
 * @omit: Alarm types to omit
 * @resolve_tzid: (closure user_data) (scope call): Callback for resolving
 * timezones
 * @user_data: (closure): Data to be passed to the resolve_tzid callback
 * @default_timezone: The timezone used to resolve DATE and floating DATE-TIME
 * values.
 *
 * Generates alarm instances for a calendar component.  Returns the instances
 * structure, or %NULL if no alarm instances occurred in the specified time
 * range.
 *
 * Returns: (allow-none) (transfer full): a list of all the alarms found for the
 * given component in the given time range. The list of alarms should be freed
 * by using e_cal_component_free_alarm_list().
 */
ECalComponentAlarms *
e_cal_util_generate_alarms_for_comp (ECalComponent *comp,
                                     time_t start,
                                     time_t end,
                                     ECalComponentAlarmAction *omit,
                                     ECalRecurResolveTimezoneFn resolve_tzid,
                                     gpointer user_data,
                                     icaltimezone *default_timezone)
{
	GList *alarm_uids;
	time_t alarm_start, alarm_end;
	struct alarm_occurrence_data aod;
	ECalComponentAlarms *alarms;

	if (!e_cal_component_has_alarms (comp))
		return NULL;

	alarm_uids = e_cal_component_get_alarm_uids (comp);
	compute_alarm_range (
		comp, alarm_uids, start, end, &alarm_start, &alarm_end);

	aod.alarm_uids = alarm_uids;
	aod.start = start;
	aod.end = end;
	aod.omit = omit;
	aod.triggers = NULL;
	aod.n_triggers = 0;

	e_cal_recur_generate_instances (
		comp, alarm_start, alarm_end,
		add_alarm_occurrences_cb, &aod,
		resolve_tzid, user_data,
		default_timezone);

	/* We add the ABSOLUTE triggers separately */
	generate_absolute_triggers (
		comp, &aod, resolve_tzid, user_data, default_timezone);

	cal_obj_uid_list_free (alarm_uids);

	if (aod.n_triggers == 0)
		return NULL;

	/* Create the component alarm instances structure */

	alarms = g_new (ECalComponentAlarms, 1);
	alarms->comp = comp;
	g_object_ref (G_OBJECT (alarms->comp));
	alarms->alarms = g_slist_sort (aod.triggers, compare_alarm_instance);

	return alarms;
}

/**
 * e_cal_util_generate_alarms_for_list:
 * @comps: (element-type ECalComponent): List of #ECalComponent<!-- -->s
 * @start: Start time
 * @end: End time
 * @omit: Alarm types to omit
 * @comp_alarms: (out) (transfer full) (element-type ECalComponentAlarms): List
 * to be returned
 * @resolve_tzid: (closure user_data) (scope call): Callback for resolving
 * timezones
 * @user_data: (closure): Data to be passed to the resolve_tzid callback
 * @default_timezone: The timezone used to resolve DATE and floating DATE-TIME
 * values.
 *
 * Iterates through all the components in the @comps list and generates alarm
 * instances for them; putting them in the @comp_alarms list.
 *
 * Returns: the number of elements it added to the list
 */
gint
e_cal_util_generate_alarms_for_list (GList *comps,
                                     time_t start,
                                     time_t end,
                                     ECalComponentAlarmAction *omit,
                                     GSList **comp_alarms,
                                     ECalRecurResolveTimezoneFn resolve_tzid,
                                     gpointer user_data,
                                     icaltimezone *default_timezone)
{
	GList *l;
	gint n;

	n = 0;

	for (l = comps; l; l = l->next) {
		ECalComponent *comp;
		ECalComponentAlarms *alarms;

		comp = E_CAL_COMPONENT (l->data);
		alarms = e_cal_util_generate_alarms_for_comp (
			comp, start, end, omit, resolve_tzid,
			user_data, default_timezone);

		if (alarms) {
			*comp_alarms = g_slist_prepend (*comp_alarms, alarms);
			n++;
		}
	}

	return n;
}

/**
 * e_cal_util_priority_to_string:
 * @priority: Priority value.
 *
 * Converts an iCalendar PRIORITY value to a translated string. Any unknown
 * priority value (i.e. not 0-9) will be returned as "" (undefined).
 *
 * Returns: a string representing the PRIORITY value. This value is a
 * constant, so it should never be freed.
 */
const gchar *
e_cal_util_priority_to_string (gint priority)
{
	const gchar *retval;

	if (priority <= 0)
		retval = "";
	else if (priority <= 4)
		retval = C_("Priority", "High");
	else if (priority == 5)
		retval = C_("Priority", "Normal");
	else if (priority <= 9)
		retval = C_("Priority", "Low");
	else
		retval = "";

	return retval;
}

/**
 * e_cal_util_priority_from_string:
 * @string: A string representing the PRIORITY value.
 *
 * Converts a translated priority string to an iCalendar priority value.
 *
 * Returns: the priority (0-9) or -1 if the priority string is not valid.
*/
gint
e_cal_util_priority_from_string (const gchar *string)
{
	gint priority;

	/* An empty string is the same as 'None'. */
	if (!string || !string[0] || !e_util_utf8_strcasecmp (string, C_("Priority", "Undefined")))
		priority = 0;
	else if (!e_util_utf8_strcasecmp (string, C_("Priority", "High")))
		priority = 3;
	else if (!e_util_utf8_strcasecmp (string, C_("Priority", "Normal")))
		priority = 5;
	else if (!e_util_utf8_strcasecmp (string, C_("Priority", "Low")))
		priority = 7;
	else
		priority = -1;

	return priority;
}

/* callback for icalcomponent_foreach_tzid */
typedef struct {
	icalcomponent *vcal_comp;
	icalcomponent *icalcomp;
} ForeachTzidData;

static void
add_timezone_cb (icalparameter *param,
                 gpointer data)
{
	icaltimezone *tz;
	const gchar *tzid;
	icalcomponent *vtz_comp;
	ForeachTzidData *f_data = (ForeachTzidData *) data;

	tzid = icalparameter_get_tzid (param);
	if (!tzid)
		return;

	tz = icalcomponent_get_timezone (f_data->vcal_comp, tzid);
	if (tz)
		return;

	tz = icalcomponent_get_timezone (f_data->icalcomp, tzid);
	if (!tz) {
		tz = icaltimezone_get_builtin_timezone_from_tzid (tzid);
		if (!tz)
			return;
	}

	vtz_comp = icaltimezone_get_component (tz);
	if (!vtz_comp)
		return;

	icalcomponent_add_component (
		f_data->vcal_comp,
		icalcomponent_new_clone (vtz_comp));
}

/**
 * e_cal_util_add_timezones_from_component:
 * @vcal_comp: A VCALENDAR component.
 * @icalcomp: An iCalendar component, of any type.
 *
 * Adds VTIMEZONE components to a VCALENDAR for all tzid's
 * in the given @icalcomp.
 */
void
e_cal_util_add_timezones_from_component (icalcomponent *vcal_comp,
                                         icalcomponent *icalcomp)
{
	ForeachTzidData f_data;

	g_return_if_fail (vcal_comp != NULL);
	g_return_if_fail (icalcomp != NULL);

	f_data.vcal_comp = vcal_comp;
	f_data.icalcomp = icalcomp;
	icalcomponent_foreach_tzid (icalcomp, add_timezone_cb, &f_data);
}

/**
 * e_cal_util_component_is_instance:
 * @icalcomp: An #icalcomponent.
 *
 * Checks whether an #icalcomponent is an instance of a recurring appointment.
 *
 * Returns: TRUE if it is an instance, FALSE if not.
 */
gboolean
e_cal_util_component_is_instance (icalcomponent *icalcomp)
{
	icalproperty *prop;

	g_return_val_if_fail (icalcomp != NULL, FALSE);

	prop = icalcomponent_get_first_property (
		icalcomp, ICAL_RECURRENCEID_PROPERTY);

	return (prop != NULL);
}

/**
 * e_cal_util_component_has_alarms:
 * @icalcomp: An #icalcomponent.
 *
 * Checks whether an #icalcomponent has any alarm.
 *
 * Returns: TRUE if it has alarms, FALSE otherwise.
 */
gboolean
e_cal_util_component_has_alarms (icalcomponent *icalcomp)
{
	icalcomponent *alarm;

	g_return_val_if_fail (icalcomp != NULL, FALSE);

	alarm = icalcomponent_get_first_component (
		icalcomp, ICAL_VALARM_COMPONENT);

	return (alarm != NULL);
}

/**
 * e_cal_util_component_has_organizer:
 * @icalcomp: An #icalcomponent.
 *
 * Checks whether an #icalcomponent has an organizer.
 *
 * Returns: TRUE if there is an organizer, FALSE if not.
 */
gboolean
e_cal_util_component_has_organizer (icalcomponent *icalcomp)
{
	icalproperty *prop;

	g_return_val_if_fail (icalcomp != NULL, FALSE);

	prop = icalcomponent_get_first_property (
		icalcomp, ICAL_ORGANIZER_PROPERTY);

	return (prop != NULL);
}

/**
 * e_cal_util_component_has_attendee:
 * @icalcomp: An #icalcomponent.
 *
 * Checks if an #icalcomponent has any attendees.
 *
 * Returns: TRUE if there are attendees, FALSE if not.
 */
gboolean
e_cal_util_component_has_attendee (icalcomponent *icalcomp)
{
	icalproperty *prop;

	g_return_val_if_fail (icalcomp != NULL, FALSE);

	prop = icalcomponent_get_first_property (
		icalcomp, ICAL_ATTENDEE_PROPERTY);

	return (prop != NULL);
}

/**
 * e_cal_util_component_has_recurrences:
 * @icalcomp: An #icalcomponent.
 *
 * Checks if an #icalcomponent has recurrence dates or rules.
 *
 * Returns: TRUE if there are recurrence dates/rules, FALSE if not.
 */
gboolean
e_cal_util_component_has_recurrences (icalcomponent *icalcomp)
{
	g_return_val_if_fail (icalcomp != NULL, FALSE);

	return e_cal_util_component_has_rdates (icalcomp) ||
		e_cal_util_component_has_rrules (icalcomp);
}

/**
 * e_cal_util_component_has_rdates:
 * @icalcomp: An #icalcomponent.
 *
 * Checks if an #icalcomponent has recurrence dates.
 *
 * Returns: TRUE if there are recurrence dates, FALSE if not.
 */
gboolean
e_cal_util_component_has_rdates (icalcomponent *icalcomp)
{
	icalproperty *prop;

	g_return_val_if_fail (icalcomp != NULL, FALSE);

	prop = icalcomponent_get_first_property (
		icalcomp, ICAL_RDATE_PROPERTY);

	return (prop != NULL);
}

/**
 * e_cal_util_component_has_rrules:
 * @icalcomp: An #icalcomponent.
 *
 * Checks if an #icalcomponent has recurrence rules.
 *
 * Returns: TRUE if there are recurrence rules, FALSE if not.
 */
gboolean
e_cal_util_component_has_rrules (icalcomponent *icalcomp)
{
	icalproperty *prop;

	g_return_val_if_fail (icalcomp != NULL, FALSE);

	prop = icalcomponent_get_first_property (
		icalcomp, ICAL_RRULE_PROPERTY);

	return (prop != NULL);
}

/**
 * e_cal_util_event_dates_match:
 * @icalcomp1: An #icalcomponent.
 * @icalcomp2: An #icalcomponent.
 *
 * Compare the dates of two #icalcomponent's to check if they match.
 *
 * Returns: TRUE if the dates of both components match, FALSE otherwise.
 */
gboolean
e_cal_util_event_dates_match (icalcomponent *icalcomp1,
                              icalcomponent *icalcomp2)
{
	struct icaltimetype c1_dtstart, c1_dtend, c2_dtstart, c2_dtend;

	g_return_val_if_fail (icalcomp1 != NULL, FALSE);
	g_return_val_if_fail (icalcomp2 != NULL, FALSE);

	c1_dtstart = icalcomponent_get_dtstart (icalcomp1);
	c1_dtend = icalcomponent_get_dtend (icalcomp1);
	c2_dtstart = icalcomponent_get_dtstart (icalcomp2);
	c2_dtend = icalcomponent_get_dtend (icalcomp2);

	/* if either value is NULL, they must both be NULL to match */
	if (icaltime_is_valid_time (c1_dtstart) || icaltime_is_valid_time (c2_dtstart)) {
		if (!(icaltime_is_valid_time (c1_dtstart) && icaltime_is_valid_time (c2_dtstart)))
			return FALSE;
	} else {
		if (icaltime_compare (c1_dtstart, c2_dtstart))
			return FALSE;
	}

	if (icaltime_is_valid_time (c1_dtend) || icaltime_is_valid_time (c2_dtend)) {
		if (!(icaltime_is_valid_time (c1_dtend) && icaltime_is_valid_time (c2_dtend)))
			return FALSE;
	} else {
		if (icaltime_compare (c1_dtend, c2_dtend))
			return FALSE;
	}

	/* now match the timezones */
	if (!(!c1_dtstart.zone && !c2_dtstart.zone) ||
	    (c1_dtstart.zone && c2_dtstart.zone &&
	     !strcmp (icaltimezone_get_tzid ((icaltimezone *) c1_dtstart.zone),
		      icaltimezone_get_tzid ((icaltimezone *) c2_dtstart.zone))))
		return FALSE;

	if (!(!c1_dtend.zone && !c2_dtend.zone) ||
	    (c1_dtend.zone && c2_dtend.zone &&
	     !strcmp (icaltimezone_get_tzid ((icaltimezone *) c1_dtend.zone),
		      icaltimezone_get_tzid ((icaltimezone *) c2_dtend.zone))))
		return FALSE;

	return TRUE;
}

/* Individual instances management */

struct instance_data {
	time_t start;
	gboolean found;
};

static void
check_instance (icalcomponent *comp,
                struct icaltime_span *span,
                gpointer data)
{
	struct instance_data *instance = data;

	if (span->start == instance->start)
		instance->found = TRUE;
}

/**
 * e_cal_util_construct_instance:
 * @icalcomp: A recurring #icalcomponent
 * @rid: The RECURRENCE-ID to construct a component for
 *
 * This checks that @rid indicates a valid recurrence of @icalcomp, and
 * if so, generates a copy of @comp containing a RECURRENCE-ID of @rid.
 *
 * Returns: the instance, or %NULL.
 **/
icalcomponent *
e_cal_util_construct_instance (icalcomponent *icalcomp,
                               struct icaltimetype rid)
{
	struct instance_data instance;
	struct icaltimetype start, end;

	g_return_val_if_fail (icalcomp != NULL, NULL);

	/* Make sure this is really recurring */
	if (!icalcomponent_get_first_property (icalcomp, ICAL_RRULE_PROPERTY) &&
	    !icalcomponent_get_first_property (icalcomp, ICAL_RDATE_PROPERTY))
		return NULL;

	/* Make sure the specified instance really exists */
	start = icaltime_convert_to_zone (rid, icaltimezone_get_utc_timezone ());
	end = start;
	icaltime_adjust (&end, 0, 0, 0, 1);

	instance.start = icaltime_as_timet (start);
	instance.found = FALSE;
	icalcomponent_foreach_recurrence (icalcomp, start, end,
					  check_instance, &instance);
	if (!instance.found)
		return NULL;

	/* Make the instance */
	icalcomp = icalcomponent_new_clone (icalcomp);
	icalcomponent_set_recurrenceid (icalcomp, rid);

	return icalcomp;
}

static inline gboolean
time_matches_rid (struct icaltimetype itt,
                  struct icaltimetype rid,
                  ECalObjModType mod)
{
	gint compare;

	compare = icaltime_compare (itt, rid);
	if (compare == 0)
		return TRUE;
	else if (compare < 0 && (mod & E_CAL_OBJ_MOD_THIS_AND_PRIOR))
		return TRUE;
	else if (compare > 0 && (mod & E_CAL_OBJ_MOD_THIS_AND_FUTURE))
		return TRUE;

	return FALSE;
}

static void
e_cal_util_remove_instances_ex (icalcomponent *icalcomp,
				struct icaltimetype rid,
				ECalObjModType mod,
				gboolean keep_rid,
				gboolean can_add_exrule)
{
	icalproperty *prop;
	struct icaltimetype itt, recur;
	struct icalrecurrencetype rule;
	icalrecur_iterator *iter;
	GSList *remove_props = NULL, *rrules = NULL, *link;

	g_return_if_fail (icalcomp != NULL);
	g_return_if_fail (mod != E_CAL_OBJ_MOD_ALL);

	/* First remove RDATEs and EXDATEs in the indicated range. */
	for (prop = icalcomponent_get_first_property (icalcomp, ICAL_RDATE_PROPERTY);
	     prop;
	     prop = icalcomponent_get_next_property (icalcomp, ICAL_RDATE_PROPERTY)) {
		struct icaldatetimeperiodtype period;

		period = icalproperty_get_rdate (prop);
		if (time_matches_rid (period.time, rid, mod) && (!keep_rid ||
		    icaltime_compare (period.time, rid) != 0))
			remove_props = g_slist_prepend (remove_props, prop);
	}
	for (prop = icalcomponent_get_first_property (icalcomp, ICAL_EXDATE_PROPERTY);
	     prop;
	     prop = icalcomponent_get_next_property (icalcomp, ICAL_EXDATE_PROPERTY)) {
		itt = icalproperty_get_exdate (prop);
		if (time_matches_rid (itt, rid, mod) && (!keep_rid ||
		    icaltime_compare (itt, rid) != 0))
			remove_props = g_slist_prepend (remove_props, prop);
	}

	for (link = remove_props; link; link = g_slist_next (link)) {
		prop = link->data;

		icalcomponent_remove_property (icalcomp, prop);
	}

	g_slist_free (remove_props);
	remove_props = NULL;

	/* If we're only removing one instance, just add an EXDATE. */
	if (mod == E_CAL_OBJ_MOD_THIS) {
		prop = icalproperty_new_exdate (rid);
		icalcomponent_add_property (icalcomp, prop);
		return;
	}

	/* Otherwise, iterate through RRULEs */
	/* FIXME: this may generate duplicate EXRULEs */
	for (prop = icalcomponent_get_first_property (icalcomp, ICAL_RRULE_PROPERTY);
	     prop;
	     prop = icalcomponent_get_next_property (icalcomp, ICAL_RRULE_PROPERTY)) {
		rrules = g_slist_prepend (rrules, prop);
	}

	for (link = rrules; link; link = g_slist_next (link)) {
		prop = link->data;
		rule = icalproperty_get_rrule (prop);

		iter = icalrecur_iterator_new (rule, rid);
		recur = icalrecur_iterator_next (iter);

		if (mod & E_CAL_OBJ_MOD_THIS_AND_FUTURE) {
			/* Truncate the rule at rid. */
			if (!icaltime_is_null_time (recur)) {
				/* Use count if it was used */
				if (rule.count > 0) {
					gint occurrences_count = 0;
					icalrecur_iterator *count_iter;
					struct icaltimetype count_recur;

					count_iter = icalrecur_iterator_new (rule, icalcomponent_get_dtstart (icalcomp));
					while (count_recur = icalrecur_iterator_next (count_iter), !icaltime_is_null_time (count_recur) && occurrences_count < rule.count) {
						if (icaltime_compare (count_recur, rid) >= 0)
							break;

						occurrences_count++;
					}

					icalrecur_iterator_free (count_iter);

					if (keep_rid && icaltime_compare (count_recur, rid) == 0)
						occurrences_count++;

					/* The caller should make sure that the remove will keep at least one instance */
					g_warn_if_fail (occurrences_count > 0);

					rule.count = occurrences_count;
				} else {
					if (keep_rid && icaltime_compare (recur, rid) == 0)
						rule.until = icaltime_add (rid, icalcomponent_get_duration (icalcomp));
					else
						rule.until = rid;
					icaltime_adjust (&rule.until, 0, 0, 0, -1);
				}

				icalproperty_set_rrule (prop, rule);
				icalproperty_remove_parameter_by_name (prop, "X-EVOLUTION-ENDDATE");
			}
		} else {
			/* (If recur == rid, skip to the next occurrence) */
			if (!keep_rid && icaltime_compare (recur, rid) == 0)
				recur = icalrecur_iterator_next (iter);

			/* If there is a recurrence after rid, add
			 * an EXRULE to block instances up to rid.
			 * Otherwise, just remove the RRULE.
			 */
			if (!icaltime_is_null_time (recur)) {
				if (can_add_exrule) {
					rule.count = 0;
					/* iCalendar says we should just use rid
					 * here, but Outlook/Exchange handle
					 * UNTIL incorrectly.
					 */
					if (keep_rid && icaltime_compare (recur, rid) == 0) {
						struct icaldurationtype duration = icalcomponent_get_duration (icalcomp);
						duration.is_neg = !duration.is_neg;
						rule.until = icaltime_add (rid, duration);
					} else
						rule.until = icaltime_add (rid, icalcomponent_get_duration (icalcomp));
					prop = icalproperty_new_exrule (rule);
					icalcomponent_add_property (icalcomp, prop);
				}
			} else {
				remove_props = g_slist_prepend (remove_props, prop);
			}
		}

		icalrecur_iterator_free (iter);
	}

	for (link = remove_props; link; link = g_slist_next (link)) {
		prop = link->data;

		icalcomponent_remove_property (icalcomp, prop);
	}

	g_slist_free (remove_props);
	g_slist_free (rrules);
}

/**
 * e_cal_util_remove_instances:
 * @icalcomp: A (recurring) #icalcomponent
 * @rid: The base RECURRENCE-ID to remove
 * @mod: How to interpret @rid
 *
 * Removes one or more instances from @comp according to @rid and @mod.
 *
 * FIXME: should probably have a return value indicating whether @icalcomp
 *        still has any instances
 **/
void
e_cal_util_remove_instances (icalcomponent *icalcomp,
                             struct icaltimetype rid,
                             ECalObjModType mod)
{
	g_return_if_fail (icalcomp != NULL);
	g_return_if_fail (mod != E_CAL_OBJ_MOD_ALL);

	e_cal_util_remove_instances_ex (icalcomp, rid, mod, FALSE, TRUE);
}

/**
 * e_cal_util_split_at_instance:
 * @icalcomp: A (recurring) #icalcomponent
 * @rid: The base RECURRENCE-ID to remove
 * @master_dtstart: The DTSTART of the master object
 *
 * Splits a recurring @icalcomp into two at time @rid. The returned icalcomponent
 * is modified @icalcomp which contains recurrences beginning at @rid, inclusive.
 * The instance identified by @rid should exist. The @master_dtstart can be
 * a null time, then it is read from the @icalcomp.
 *
 * Use e_cal_util_remove_instances() with E_CAL_OBJ_MOD_THIS_AND_FUTURE mode
 * on the @icalcomp to remove the overlapping interval from it, if needed.
 *
 * Returns: the split icalcomponent, or %NULL.
 *
 * Since: 3.16
 **/
icalcomponent *
e_cal_util_split_at_instance (icalcomponent *icalcomp,
			      struct icaltimetype rid,
			      struct icaltimetype master_dtstart)
{
	icalproperty *prop;
	struct instance_data instance;
	struct icaltimetype start, end;
	struct icaldurationtype duration;
	GSList *remove_props = NULL, *link;

	g_return_val_if_fail (icalcomp != NULL, NULL);
	g_return_val_if_fail (!icaltime_is_null_time (rid), NULL);

	/* Make sure this is really recurring */
	if (!icalcomponent_get_first_property (icalcomp, ICAL_RRULE_PROPERTY) &&
	    !icalcomponent_get_first_property (icalcomp, ICAL_RDATE_PROPERTY))
		return NULL;

	/* Make sure the specified instance really exists */
	start = icaltime_convert_to_zone (rid, icaltimezone_get_utc_timezone ());
	end = start;
	icaltime_adjust (&end, 0, 0, 0, 1);

	instance.start = icaltime_as_timet (start);
	instance.found = FALSE;
	icalcomponent_foreach_recurrence (icalcomp, start, end,
					  check_instance, &instance);
	/* Make the copy */
	icalcomp = icalcomponent_new_clone (icalcomp);

	e_cal_util_remove_instances_ex (icalcomp, rid, E_CAL_OBJ_MOD_THIS_AND_PRIOR, TRUE, FALSE);

	start = rid;
	if (icaltime_is_null_time (master_dtstart))
		master_dtstart = icalcomponent_get_dtstart (icalcomp);
	duration = icalcomponent_get_duration (icalcomp);

	/* Expect that DTSTART and DTEND are already set when the instance could not be found */
	if (instance.found) {
		icalcomponent_set_dtstart (icalcomp, start);
		/* Update either DURATION or DTEND */
		if (icaltime_is_null_time (icalcomponent_get_dtend (icalcomp))) {
			icalcomponent_set_duration (icalcomp, duration);
		} else {
			end = start;
			if (duration.is_neg)
				icaltime_adjust (&end, -duration.days - 7 * duration.weeks, -duration.hours, -duration.minutes, -duration.seconds);
			else
				icaltime_adjust (&end, duration.days + 7 * duration.weeks, duration.hours, duration.minutes, duration.seconds);
			icalcomponent_set_dtend (icalcomp, end);
		}
	}

	/* any RRULE with 'count' should be shortened */
	for (prop = icalcomponent_get_first_property (icalcomp, ICAL_RRULE_PROPERTY);
	     prop;
	     prop = icalcomponent_get_next_property (icalcomp, ICAL_RRULE_PROPERTY)) {
		struct icaltimetype recur;
		struct icalrecurrencetype rule;

		rule = icalproperty_get_rrule (prop);

		if (rule.count != 0) {
			gint occurrences_count = 0;
			icalrecur_iterator *iter;

			iter = icalrecur_iterator_new (rule, master_dtstart);
			while (recur = icalrecur_iterator_next (iter), !icaltime_is_null_time (recur) && occurrences_count < rule.count) {
				if (icaltime_compare (recur, rid) >= 0)
					break;

				occurrences_count++;
			}

			icalrecur_iterator_free (iter);

			if (icaltime_is_null_time (recur)) {
				remove_props = g_slist_prepend (remove_props, prop);
			} else {
				rule.count -= occurrences_count;
				icalproperty_set_rrule (prop, rule);
				icalproperty_remove_parameter_by_name (prop, "X-EVOLUTION-ENDDATE");
			}
		}
	}

	for (link = remove_props; link; link = g_slist_next (link)) {
		prop = link->data;

		icalcomponent_remove_property (icalcomp, prop);
	}

	g_slist_free (remove_props);

	return icalcomp;
}

typedef struct {
	struct icaltimetype rid;
	gboolean matches;
} CheckFirstInstanceData;

static gboolean
check_first_instance_cb (ECalComponent *comp,
			 time_t instance_start,
			 time_t instance_end,
			 gpointer user_data)
{
	CheckFirstInstanceData *ifs = user_data;
	icalcomponent *icalcomp;
	struct icaltimetype rid;

	g_return_val_if_fail (ifs != NULL, FALSE);

	icalcomp = e_cal_component_get_icalcomponent (comp);
	if (icalcomponent_get_first_property (icalcomp, ICAL_RECURRENCEID_PROPERTY) != NULL) {
		rid = icalcomponent_get_recurrenceid (icalcomp);
	} else {
		struct icaltimetype dtstart;

		dtstart = icalcomponent_get_dtstart (icalcomp);
		rid = icaltime_from_timet_with_zone (instance_start, dtstart.is_date, dtstart.zone);
	}

	ifs->matches = icaltime_compare (ifs->rid, rid) == 0;

	return FALSE;
}

/**
 * e_cal_util_is_first_instance:
 * @comp: an #ECalComponent instance
 * @rid: a recurrence ID
 * @tz_cb: (closure tz_cb_data) (scope call): The #ECalRecurResolveTimezoneFn to call
 * @tz_cb_data: (closure): User data to be passed to the @tz_cb callback
 *
 * Returns whether the given @rid is the first instance of
 * the recurrence defined in the @comp.
 *
 * Return: Whether the @rid identifies the first instance of @comp.
 *
 * Since: 3.16
 **/
gboolean
e_cal_util_is_first_instance (ECalComponent *comp,
			      struct icaltimetype rid,
			      ECalRecurResolveTimezoneFn tz_cb,
			      gpointer tz_cb_data)
{
	CheckFirstInstanceData ifs;
	icalcomponent *icalcomp;
	time_t start, end;

	g_return_val_if_fail (E_IS_CAL_COMPONENT (comp), FALSE);
	g_return_val_if_fail (!icaltime_is_null_time (rid), FALSE);

	ifs.rid = rid;
	ifs.matches = FALSE;

	icalcomp = e_cal_component_get_icalcomponent (comp);
	start = icaltime_as_timet (icalcomponent_get_dtstart (icalcomp)) - 24 * 60 * 60;
	end = icaltime_as_timet (icalcomponent_get_dtend (icalcomp)) + 24 * 60 * 60;

	e_cal_recur_generate_instances (comp, start, end, check_first_instance_cb, &ifs,
		tz_cb, tz_cb_data, icaltimezone_get_utc_timezone ());

	return ifs.matches;
}

/**
 * e_cal_util_get_system_timezone_location:
 *
 * Fetches system timezone localtion string.
 *
 * Returns: (transfer full): system timezone location string, %NULL on an error.
 *
 * Since: 2.28
 **/
gchar *
e_cal_util_get_system_timezone_location (void)
{
	return e_cal_system_timezone_get_location ();
}

/**
 * e_cal_util_get_system_timezone:
 *
 * Fetches system timezone icaltimezone object.
 *
 * The returned pointer is part of the built-in timezones and should not be freed.
 *
 * Returns: (transfer none): The icaltimezone object of the system timezone, or %NULL on an error.
 *
 * Since: 2.28
 **/
icaltimezone *
e_cal_util_get_system_timezone (void)
{
	gchar *location;
	icaltimezone *zone;

	location = e_cal_system_timezone_get_location ();
	g_return_val_if_fail (location != NULL, NULL);

	zone = icaltimezone_get_builtin_timezone (location);

	g_free (location);

	return zone;
}

static time_t
componenttime_to_utc_timet (const ECalComponentDateTime *dt_time,
                            ECalRecurResolveTimezoneFn tz_cb,
                            gpointer tz_cb_data,
                            const icaltimezone *default_zone)
{
	time_t timet = -1;
	icaltimezone *zone = NULL;

	g_return_val_if_fail (dt_time != NULL, -1);

	if (dt_time->value) {
		if (dt_time->tzid)
			zone = tz_cb (dt_time->tzid, tz_cb_data);

		// zone = icaltimezone_get_utc_timezone ();
		timet = icaltime_as_timet_with_zone (
			*dt_time->value, zone ? zone : default_zone);
	}

	return timet;
}

/**
 * e_cal_util_get_component_occur_times:
 * @comp: an #ECalComponent
 * @start: (out): Location to store the start time
 * @end: (out): Location to store the end time
 * @tz_cb: (closure tz_cb_data) (scope call): The #ECalRecurResolveTimezoneFn to call
 * @tz_cb_data: (closure): User data to be passed to the @tz_cb callback
 * @default_timezone: The default timezone
 * @kind: the type of component, indicated with an icalcomponent_kind
 *
 * Find out when the component starts and stops, being careful about
 * recurrences.
 *
 * Since: 2.32
 **/
void
e_cal_util_get_component_occur_times (ECalComponent *comp,
                                      time_t *start,
                                      time_t *end,
                                      ECalRecurResolveTimezoneFn tz_cb,
                                      gpointer tz_cb_data,
                                      const icaltimezone *default_timezone,
                                      icalcomponent_kind kind)
{
	struct icalrecurrencetype ir;
	ECalComponentDateTime dt_start, dt_end;

	g_return_if_fail (comp != NULL);
	g_return_if_fail (start != NULL);
	g_return_if_fail (end != NULL);

	e_cal_recur_ensure_end_dates (comp, FALSE, tz_cb, tz_cb_data);

	/* Get dtstart of the component and convert it to UTC */
	e_cal_component_get_dtstart (comp, &dt_start);

	if ((*start = componenttime_to_utc_timet (&dt_start, tz_cb, tz_cb_data, default_timezone)) == -1)
		*start = _TIME_MIN;

	e_cal_component_free_datetime (&dt_start);

	/* find out end date of component */
	*end = _TIME_MAX;

	if (kind == ICAL_VTODO_COMPONENT) {
		/* max from COMPLETED and DUE properties */
		struct icaltimetype *tt = NULL;
		time_t completed_time = -1, due_time = -1, max_time;
		ECalComponentDateTime dt_due;

		e_cal_component_get_completed (comp, &tt);
		if (tt) {
			/* COMPLETED must be in UTC. */
			completed_time = icaltime_as_timet_with_zone (
				*tt, icaltimezone_get_utc_timezone ());
			e_cal_component_free_icaltimetype (tt);
		}

		e_cal_component_get_due (comp, &dt_due);
		if (dt_due.value != NULL)
			due_time = componenttime_to_utc_timet (
				&dt_due, tz_cb, tz_cb_data,
				default_timezone);

		e_cal_component_free_datetime (&dt_due);

		max_time = MAX (completed_time, due_time);

		if (max_time != -1)
			*end = max_time;

	} else {
		/* ALARMS, EVENTS: DTEND and reccurences */

		if (e_cal_component_has_recurrences (comp)) {
			GSList *rrules = NULL;
			GSList *exrules = NULL;
			GSList *elem;
			GSList *rdates = NULL;

			/* Do the RRULEs, EXRULEs and RDATEs*/
			e_cal_component_get_rrule_property_list (comp, &rrules);
			e_cal_component_get_exrule_property_list (comp, &exrules);
			e_cal_component_get_rdate_list (comp, &rdates);

			for (elem = rrules; elem; elem = elem->next) {
				time_t rule_end;
				icaltimezone *utc_zone;
				icalproperty *prop = elem->data;
				ir = icalproperty_get_rrule (prop);

				utc_zone = icaltimezone_get_utc_timezone ();
				rule_end = e_cal_recur_obtain_enddate (
					&ir, prop, utc_zone, TRUE);

				if (rule_end == -1) /* repeats forever */
					*end = _TIME_MAX;
				else if (rule_end > *end) /* new maximum */
					*end = rule_end;
			}

			/* Do the EXRULEs. */
			for (elem = exrules; elem; elem = elem->next) {
				icalproperty *prop = elem->data;
				time_t rule_end;
				icaltimezone *utc_zone;
				ir = icalproperty_get_exrule (prop);

				utc_zone = icaltimezone_get_utc_timezone ();
				rule_end = e_cal_recur_obtain_enddate (
					&ir, prop, utc_zone, TRUE);

				if (rule_end == -1) /* repeats forever */
					*end = _TIME_MAX;
				else if (rule_end > *end)
					*end = rule_end;
			}

			/* Do the RDATEs */
			for (elem = rdates; elem; elem = elem->next) {
				ECalComponentPeriod *p = elem->data;
				time_t rdate_end = _TIME_MAX;

				/* FIXME: We currently assume RDATEs are in the same timezone
				 * as DTSTART. We should get the RDATE timezone and convert
				 * to the DTSTART timezone first. */

				/* Check if the end date or duration is set, libical seems to set
				 * second to -1 to denote an unset time */
				if (p->type != E_CAL_COMPONENT_PERIOD_DATETIME || p->u.end.second != -1)
					rdate_end = icaltime_as_timet (icaltime_add (p->start, p->u.duration));
				else
					rdate_end = icaltime_as_timet (p->u.end);

				if (rdate_end == -1) /* repeats forever */
					*end = _TIME_MAX;
				else if (rdate_end > *end)
					*end = rdate_end;
			}

			e_cal_component_free_period_list (rdates);
		}

		/* Get dtend of the component and convert it to UTC */
		e_cal_component_get_dtend (comp, &dt_end);

		if (dt_end.value) {
			time_t dtend_time;

			dtend_time = componenttime_to_utc_timet (
				&dt_end, tz_cb, tz_cb_data, default_timezone);

			if (dtend_time == -1 || (dtend_time > *end))
				*end = dtend_time;
		}

		e_cal_component_free_datetime (&dt_end);
	}
}
