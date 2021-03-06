/*
 * camel-nntp-settings.c
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

#include "camel-nntp-settings.h"

#define CAMEL_NNTP_SETTINGS_GET_PRIVATE(obj) \
	(G_TYPE_INSTANCE_GET_PRIVATE \
	((obj), CAMEL_TYPE_NNTP_SETTINGS, CamelNNTPSettingsPrivate))

struct _CamelNNTPSettingsPrivate {
	gboolean filter_all;
	gboolean folder_hierarchy_relative;
	gboolean short_folder_names;
};

enum {
	PROP_0,
	PROP_AUTH_MECHANISM,
	PROP_FILTER_ALL,
	PROP_FOLDER_HIERARCHY_RELATIVE,
	PROP_HOST,
	PROP_PORT,
	PROP_SECURITY_METHOD,
	PROP_SHORT_FOLDER_NAMES,
	PROP_USER
};

G_DEFINE_TYPE_WITH_CODE (
	CamelNNTPSettings,
	camel_nntp_settings,
	CAMEL_TYPE_OFFLINE_SETTINGS,
	G_IMPLEMENT_INTERFACE (
		CAMEL_TYPE_NETWORK_SETTINGS, NULL))

static void
nntp_settings_set_property (GObject *object,
                            guint property_id,
                            const GValue *value,
                            GParamSpec *pspec)
{
	switch (property_id) {
		case PROP_AUTH_MECHANISM:
			camel_network_settings_set_auth_mechanism (
				CAMEL_NETWORK_SETTINGS (object),
				g_value_get_string (value));
			return;

		case PROP_FILTER_ALL:
			camel_nntp_settings_set_filter_all (
				CAMEL_NNTP_SETTINGS (object),
				g_value_get_boolean (value));
			return;

		case PROP_FOLDER_HIERARCHY_RELATIVE:
			camel_nntp_settings_set_folder_hierarchy_relative (
				CAMEL_NNTP_SETTINGS (object),
				g_value_get_boolean (value));
			return;

		case PROP_HOST:
			camel_network_settings_set_host (
				CAMEL_NETWORK_SETTINGS (object),
				g_value_get_string (value));
			return;

		case PROP_PORT:
			camel_network_settings_set_port (
				CAMEL_NETWORK_SETTINGS (object),
				g_value_get_uint (value));
			return;

		case PROP_SECURITY_METHOD:
			camel_network_settings_set_security_method (
				CAMEL_NETWORK_SETTINGS (object),
				g_value_get_enum (value));
			return;

		case PROP_SHORT_FOLDER_NAMES:
			camel_nntp_settings_set_short_folder_names (
				CAMEL_NNTP_SETTINGS (object),
				g_value_get_boolean (value));
			return;

		case PROP_USER:
			camel_network_settings_set_user (
				CAMEL_NETWORK_SETTINGS (object),
				g_value_get_string (value));
			return;
	}

	G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
}

static void
nntp_settings_get_property (GObject *object,
                            guint property_id,
                            GValue *value,
                            GParamSpec *pspec)
{
	switch (property_id) {
		case PROP_AUTH_MECHANISM:
			g_value_take_string (
				value,
				camel_network_settings_dup_auth_mechanism (
				CAMEL_NETWORK_SETTINGS (object)));
			return;

		case PROP_FILTER_ALL:
			g_value_set_boolean (
				value,
				camel_nntp_settings_get_filter_all (
				CAMEL_NNTP_SETTINGS (object)));
			return;

		case PROP_FOLDER_HIERARCHY_RELATIVE:
			g_value_set_boolean (
				value,
				camel_nntp_settings_get_folder_hierarchy_relative (
				CAMEL_NNTP_SETTINGS (object)));
			return;

		case PROP_HOST:
			g_value_take_string (
				value,
				camel_network_settings_dup_host (
				CAMEL_NETWORK_SETTINGS (object)));
			return;

		case PROP_PORT:
			g_value_set_uint (
				value,
				camel_network_settings_get_port (
				CAMEL_NETWORK_SETTINGS (object)));
			return;

		case PROP_SECURITY_METHOD:
			g_value_set_enum (
				value,
				camel_network_settings_get_security_method (
				CAMEL_NETWORK_SETTINGS (object)));
			return;

		case PROP_SHORT_FOLDER_NAMES:
			g_value_set_boolean (
				value,
				camel_nntp_settings_get_short_folder_names (
				CAMEL_NNTP_SETTINGS (object)));
			return;

		case PROP_USER:
			g_value_take_string (
				value,
				camel_network_settings_dup_user (
				CAMEL_NETWORK_SETTINGS (object)));
			return;
	}

	G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
}

static void
camel_nntp_settings_class_init (CamelNNTPSettingsClass *class)
{
	GObjectClass *object_class;

	g_type_class_add_private (class, sizeof (CamelNNTPSettingsPrivate));

	object_class = G_OBJECT_CLASS (class);
	object_class->set_property = nntp_settings_set_property;
	object_class->get_property = nntp_settings_get_property;

	/* Inherited from CamelNetworkSettings. */
	g_object_class_override_property (
		object_class,
		PROP_AUTH_MECHANISM,
		"auth-mechanism");

	g_object_class_install_property (
		object_class,
		PROP_FOLDER_HIERARCHY_RELATIVE,
		g_param_spec_boolean (
			"folder-hierarchy-relative",
			"Folder Hierarchy Relative",
			"Show relative folder names when subscribing",
			FALSE,
			G_PARAM_READWRITE |
			G_PARAM_CONSTRUCT |
			G_PARAM_STATIC_STRINGS));

	/* Inherited from CamelNetworkSettings. */
	g_object_class_override_property (
		object_class,
		PROP_HOST,
		"host");

	/* Inherited from CamelNetworkSettings. */
	g_object_class_override_property (
		object_class,
		PROP_PORT,
		"port");

	/* Inherited from CamelNetworkSettings. */
	g_object_class_override_property (
		object_class,
		PROP_SECURITY_METHOD,
		"security-method");

	g_object_class_install_property (
		object_class,
		PROP_SHORT_FOLDER_NAMES,
		g_param_spec_boolean (
			"short-folder-names",
			"Short Folder Names",
			"Use shortened folder names",
			FALSE,
			G_PARAM_READWRITE |
			G_PARAM_CONSTRUCT |
			G_PARAM_STATIC_STRINGS));

	/* Inherited from CamelNetworkSettings. */
	g_object_class_override_property (
		object_class,
		PROP_USER,
		"user");

	g_object_class_install_property (
		object_class,
		PROP_FILTER_ALL,
		g_param_spec_boolean (
			"filter-all",
			"Filter All",
			"Whether to apply filters in all folders",
			FALSE,
			G_PARAM_READWRITE |
			G_PARAM_CONSTRUCT |
			G_PARAM_STATIC_STRINGS));
}

static void
camel_nntp_settings_init (CamelNNTPSettings *settings)
{
	settings->priv = CAMEL_NNTP_SETTINGS_GET_PRIVATE (settings);
}

/**
 * camel_nntp_settings_get_filter_all:
 * @settings: a #CamelNNTPSettings
 *
 * Returns whether apply filters in all folders.
 *
 * Returns: whether to apply filters in all folders
 *
 * Since: 3.4
 **/
gboolean
camel_nntp_settings_get_filter_all (CamelNNTPSettings *settings)
{
	g_return_val_if_fail (CAMEL_IS_NNTP_SETTINGS (settings), FALSE);

	return settings->priv->filter_all;
}

/**
 * camel_nntp_settings_set_filter_all:
 * @settings: a #CamelNNTPSettings
 * @filter_all: whether to apply filters in all folders
 *
 * Sets whether to apply filters in all folders.
 *
 * Since: 3.4
 **/
void
camel_nntp_settings_set_filter_all (CamelNNTPSettings *settings,
                                    gboolean filter_all)
{
	g_return_if_fail (CAMEL_IS_NNTP_SETTINGS (settings));

	if (settings->priv->filter_all == filter_all)
		return;

	settings->priv->filter_all = filter_all;

	g_object_notify (G_OBJECT (settings), "filter-all");
}

/**
 * camel_nntp_settings_get_folder_hierarchy_relative:
 * @settings: a #CamelNNTPSettings
 *
 * Returns whether to show relative folder names when allowing users to
 * subscribe to folders.  Since newsgroup folder names reveal the absolute
 * path to the folder (e.g. comp.os.linux), displaying the full folder name
 * in a complete hierarchical listing of the news server is redundant, but
 * possibly harder to read.
 *
 * Returns: whether to show relative folder names
 *
 * Since: 3.2
 **/
gboolean
camel_nntp_settings_get_folder_hierarchy_relative (CamelNNTPSettings *settings)
{
	g_return_val_if_fail (CAMEL_IS_NNTP_SETTINGS (settings), FALSE);

	return settings->priv->folder_hierarchy_relative;
}

/**
 * camel_nntp_settings_set_folder_hierarchy_relative:
 * @settings: a #CamelNNTPSettings
 * @folder_hierarchy_relative: whether to show relative folder names
 *
 * Sets whether to show relative folder names when allowing users to
 * subscribe to folders.  Since newsgroup folder names reveal the absolute
 * path to the folder (e.g. comp.os.linux), displaying the full folder name
 * in a complete hierarchical listing of the news server is redundant, but
 * possibly harder to read.
 *
 * Since: 3.2
 **/
void
camel_nntp_settings_set_folder_hierarchy_relative (CamelNNTPSettings *settings,
                                                   gboolean folder_hierarchy_relative)
{
	g_return_if_fail (CAMEL_IS_NNTP_SETTINGS (settings));

	if (settings->priv->folder_hierarchy_relative == folder_hierarchy_relative)
		return;

	settings->priv->folder_hierarchy_relative = folder_hierarchy_relative;

	g_object_notify (G_OBJECT (settings), "folder-hierarchy-relative");
}

/**
 * camel_nntp_settings_get_short_folder_names:
 * @settings: a #CamelNNTPSettings
 *
 * Returns whether to use shortened folder names (e.g. c.o.linux rather
 * than comp.os.linux).
 *
 * Returns: whether to show shortened folder names
 *
 * Since: 3.2
 **/
gboolean
camel_nntp_settings_get_short_folder_names (CamelNNTPSettings *settings)
{
	g_return_val_if_fail (CAMEL_IS_NNTP_SETTINGS (settings), FALSE);

	return settings->priv->short_folder_names;
}

/**
 * camel_nntp_settings_set_short_folder_names:
 * @settings: a #CamelNNTPSettings
 * @short_folder_names: whether to show shortened folder names
 *
 * Sets whether to show shortened folder names (e.g. c.o.linux rather than
 * comp.os.linux).
 *
 * Since: 3.2
 **/
void
camel_nntp_settings_set_short_folder_names (CamelNNTPSettings *settings,
                                            gboolean short_folder_names)
{
	g_return_if_fail (CAMEL_IS_NNTP_SETTINGS (settings));

	if (settings->priv->short_folder_names == short_folder_names)
		return;

	settings->priv->short_folder_names = short_folder_names;

	g_object_notify (G_OBJECT (settings), "short-folder-names");
}

