/*
 * camel-offline-settings.c
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
 */

#include "camel-offline-settings.h"

#include <camel/camel-store-settings.h>

#define CAMEL_OFFLINE_SETTINGS_GET_PRIVATE(obj) \
	(G_TYPE_INSTANCE_GET_PRIVATE \
	((obj), CAMEL_TYPE_OFFLINE_SETTINGS, CamelOfflineSettingsPrivate))

struct _CamelOfflineSettingsPrivate {
	gboolean stay_synchronized;
	gint store_changes_interval;
};

enum {
	PROP_0,
	PROP_STAY_SYNCHRONIZED,
	PROP_STORE_CHANGES_INTERVAL
};

G_DEFINE_TYPE (
	CamelOfflineSettings,
	camel_offline_settings,
	CAMEL_TYPE_STORE_SETTINGS)

static void
offline_settings_set_property (GObject *object,
                             guint property_id,
                             const GValue *value,
                             GParamSpec *pspec)
{
	switch (property_id) {
		case PROP_STAY_SYNCHRONIZED:
			camel_offline_settings_set_stay_synchronized (
				CAMEL_OFFLINE_SETTINGS (object),
				g_value_get_boolean (value));
			return;

		case PROP_STORE_CHANGES_INTERVAL:
			camel_offline_settings_set_store_changes_interval (
				CAMEL_OFFLINE_SETTINGS (object),
				g_value_get_int (value));
			return;
	}

	G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
}

static void
offline_settings_get_property (GObject *object,
                             guint property_id,
                             GValue *value,
                             GParamSpec *pspec)
{
	switch (property_id) {
		case PROP_STAY_SYNCHRONIZED:
			g_value_set_boolean (
				value,
				camel_offline_settings_get_stay_synchronized (
				CAMEL_OFFLINE_SETTINGS (object)));
			return;

		case PROP_STORE_CHANGES_INTERVAL:
			g_value_set_int (
				value,
				camel_offline_settings_get_store_changes_interval (
				CAMEL_OFFLINE_SETTINGS (object)));
			return;
	}

	G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
}

static void
camel_offline_settings_class_init (CamelOfflineSettingsClass *class)
{
	GObjectClass *object_class;

	g_type_class_add_private (class, sizeof (CamelOfflineSettingsPrivate));

	object_class = G_OBJECT_CLASS (class);
	object_class->set_property = offline_settings_set_property;
	object_class->get_property = offline_settings_get_property;

	g_object_class_install_property (
		object_class,
		PROP_STAY_SYNCHRONIZED,
		g_param_spec_boolean (
			"stay-synchronized",
			"Stay Synchronized",
			"Stay synchronized with the remote server",
			FALSE,
			G_PARAM_READWRITE |
			G_PARAM_CONSTRUCT |
			G_PARAM_STATIC_STRINGS));

	g_object_class_install_property (
		object_class,
		PROP_STORE_CHANGES_INTERVAL,
		g_param_spec_int (
			"store-changes-interval",
			"Store Changes Interval",
			"Interval, in seconds, to store folder changes",
			G_MININT,
			G_MAXINT,
			3,
			G_PARAM_READWRITE |
			G_PARAM_CONSTRUCT |
			G_PARAM_STATIC_STRINGS));
}

static void
camel_offline_settings_init (CamelOfflineSettings *settings)
{
	settings->priv = CAMEL_OFFLINE_SETTINGS_GET_PRIVATE (settings);
}

/**
 * camel_offline_settings_get_stay_synchronized:
 * @settings: a #CamelOfflineSettings
 *
 * Returns whether to synchronize the local cache with the remote server
 * before switching to offline mode, so the store's content can still be
 * read while offline.
 *
 * Returns: whether to stay synchronized with the remote server
 *
 * Since: 3.2
 **/
gboolean
camel_offline_settings_get_stay_synchronized (CamelOfflineSettings *settings)
{
	g_return_val_if_fail (CAMEL_IS_OFFLINE_SETTINGS (settings), FALSE);

	return settings->priv->stay_synchronized;
}

/**
 * camel_offline_settings_set_stay_synchronized:
 * @settings: a #CamelOfflineSettings
 * @stay_synchronized: whether to stay synchronized with the remote server
 *
 * Sets whether to synchronize the local cache with the remote server before
 * switching to offline mode, so the store's content can still be read while
 * offline.
 *
 * Since: 3.2
 **/
void
camel_offline_settings_set_stay_synchronized (CamelOfflineSettings *settings,
                                              gboolean stay_synchronized)
{
	g_return_if_fail (CAMEL_IS_OFFLINE_SETTINGS (settings));

	if (settings->priv->stay_synchronized == stay_synchronized)
		return;

	settings->priv->stay_synchronized = stay_synchronized;

	g_object_notify (G_OBJECT (settings), "stay-synchronized");
}
/**
 * camel_offline_settings_get_store_changes_interval:
 * @settings: a #CamelOfflineSettings
 *
 * Returns the interval, in seconds, for the changes in the folder being
 * saved automatically. 0 means immediately, while -1 means turning off
 * automatic folder change saving.
 *
 * Returns: the interval for automatic store of folder changes
 *
 * Since: 3.18
 **/

gint
camel_offline_settings_get_store_changes_interval (CamelOfflineSettings *settings)
{
	g_return_val_if_fail (CAMEL_IS_OFFLINE_SETTINGS (settings), -1);

	return settings->priv->store_changes_interval;
}

/**
 * camel_offline_settings_set_store_changes_interval:
 * @settings: a #CamelOfflineSettings
 * @interval: the interval, in seconds
 *
 * Sets the interval, in seconds, for the changes in the folder being
 * saved automatically. 0 means immediately, while -1 means turning off
 * automatic folder change saving.
 *
 * Since: 3.18
 **/
void
camel_offline_settings_set_store_changes_interval (CamelOfflineSettings *settings,
						   gint interval)
{
	g_return_if_fail (CAMEL_IS_OFFLINE_SETTINGS (settings));

	if (settings->priv->store_changes_interval == interval)
		return;

	settings->priv->store_changes_interval = interval;

	g_object_notify (G_OBJECT (settings), "store-changes-interval");
}
