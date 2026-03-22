#include "color-filter.h"

#include <obs-module.h>

#define SETTING_BRIGHTNESS "brightness"
#define SETTING_CONTRAST "contrast"
#define SETTING_SATURATION "saturation"

struct color_filter_data {
	obs_source_t *context;
	gs_effect_t *effect;
	gs_eparam_t *param_brightness;
	gs_eparam_t *param_contrast;
	gs_eparam_t *param_saturation;
	float brightness;
	float contrast;
	float saturation;
};

static const char *color_filter_name(void *unused)
{
	UNUSED_PARAMETER(unused);
	return obs_module_text("SimpleColorFilter");
}

static void *color_filter_create(obs_data_t *settings, obs_source_t *source)
{
	struct color_filter_data *filter = bzalloc(sizeof(struct color_filter_data));
	filter->context = source;

	char *effect_path = obs_module_file("color_filter.effect");

	obs_enter_graphics();
	filter->effect = gs_effect_create_from_file(effect_path, NULL);
	obs_leave_graphics();

	bfree(effect_path);

	if (!filter->effect) {
		bfree(filter);
		return NULL;
	}

	filter->param_brightness = gs_effect_get_param_by_name(filter->effect, SETTING_BRIGHTNESS);
	filter->param_contrast = gs_effect_get_param_by_name(filter->effect, SETTING_CONTRAST);
	filter->param_saturation = gs_effect_get_param_by_name(filter->effect, SETTING_SATURATION);

	obs_source_update(source, settings);
	return filter;
}

static void color_filter_destroy(void *data)
{
	struct color_filter_data *filter = data;

	obs_enter_graphics();
	gs_effect_destroy(filter->effect);
	obs_leave_graphics();

	bfree(filter);
}

static void color_filter_update(void *data, obs_data_t *settings)
{
	struct color_filter_data *filter = data;
	filter->brightness = (float)obs_data_get_double(settings, SETTING_BRIGHTNESS);
	filter->contrast = (float)obs_data_get_double(settings, SETTING_CONTRAST);
	filter->saturation = (float)obs_data_get_double(settings, SETTING_SATURATION);
}

static void color_filter_render(void *data, gs_effect_t *effect)
{
	UNUSED_PARAMETER(effect);
	struct color_filter_data *filter = data;

	if (!obs_source_process_filter_begin(filter->context, GS_RGBA, OBS_ALLOW_DIRECT_RENDERING))
		return;

	gs_effect_set_float(filter->param_brightness, filter->brightness);
	gs_effect_set_float(filter->param_contrast, filter->contrast);
	gs_effect_set_float(filter->param_saturation, filter->saturation);

	obs_source_process_filter_end(filter->context, filter->effect, 0, 0);
}

static obs_properties_t *color_filter_properties(void *unused)
{
	UNUSED_PARAMETER(unused);
	obs_properties_t *props = obs_properties_create();
	obs_properties_add_float_slider(props, SETTING_BRIGHTNESS, obs_module_text("Brightness"), -1.0, 1.0, 0.01);
	obs_properties_add_float_slider(props, SETTING_CONTRAST, obs_module_text("Contrast"), 0.0, 2.0, 0.01);
	obs_properties_add_float_slider(props, SETTING_SATURATION, obs_module_text("Saturation"), 0.0, 2.0, 0.01);
	return props;
}

static void color_filter_defaults(obs_data_t *settings)
{
	obs_data_set_default_double(settings, SETTING_BRIGHTNESS, 0.0);
	obs_data_set_default_double(settings, SETTING_CONTRAST, 1.0);
	obs_data_set_default_double(settings, SETTING_SATURATION, 1.0);
}

struct obs_source_info color_filter_info = {
	.id = "simple_color_filter",
	.type = OBS_SOURCE_TYPE_FILTER,
	.output_flags = OBS_SOURCE_VIDEO,
	.get_name = color_filter_name,
	.create = color_filter_create,
	.destroy = color_filter_destroy,
	.update = color_filter_update,
	.video_render = color_filter_render,
	.get_properties = color_filter_properties,
	.get_defaults = color_filter_defaults,
};
