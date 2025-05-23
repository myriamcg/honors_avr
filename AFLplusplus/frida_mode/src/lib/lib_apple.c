#ifdef __APPLE__
  #include "frida-gumjs.h"

  #include "lib.h"
  #include "util.h"

extern mach_port_t mach_task_self();
extern void        gum_darwin_enumerate_modules(mach_port_t        task,
                                                GumFoundModuleFunc func,
                                                gpointer           user_data);

static guint64 text_base = 0;
static guint64 text_limit = 0;

  #ifdef GUM_16_6_PLUS
static gboolean lib_get_main_module(GumModule *module, gpointer user_data) {

  GumDarwinModule     **ret = (GumDarwinModule **)user_data;
  const gchar          *path = gum_module_get_path(module);
  const GumMemoryRange *range = gum_module_get_range(module);
  GumDarwinModule      *darwin_module = gum_darwin_module_new_from_memory(
      path, mach_task_self(), range->base_address, GUM_DARWIN_MODULE_FLAGS_NONE,
      NULL);

  FVERBOSE("Found main module: %s", darwin_module->name);

  *ret = darwin_module;

  return FALSE;

}

  #else
static gboolean lib_get_main_module(const GumModuleDetails *details,
                                    gpointer                user_data) {

  GumDarwinModule **ret = (GumDarwinModule **)user_data;
  GumDarwinModule  *module = gum_darwin_module_new_from_memory(
      details->path, mach_task_self(), details->range->base_address,
      GUM_DARWIN_MODULE_FLAGS_NONE, NULL);

  FVERBOSE("Found main module: %s", module->name);

  *ret = module;

  return FALSE;

}

  #endif

gboolean lib_get_text_section(const GumDarwinSectionDetails *details,
                              gpointer                       user_data) {

  UNUSED_PARAMETER(user_data);
  static size_t idx = 0;
  char          text_name[] = "__text";

  FVERBOSE("\t%2lu - base: 0x%016" G_GINT64_MODIFIER
           "X size: 0x%016" G_GINT64_MODIFIER "X %s",
           idx++, details->vm_address, details->vm_address + details->size,
           details->section_name);

  if (memcmp(details->section_name, text_name, sizeof(text_name)) == 0 &&
      text_base == 0) {

    text_base = details->vm_address;
    text_limit = details->vm_address + details->size;

  }

  FVERBOSE(".text\n");
  FVERBOSE("\taddr: 0x%016" G_GINT64_MODIFIER "X", text_base);
  FVERBOSE("\tlimit: 0x%016" G_GINT64_MODIFIER "X", text_limit);

  return TRUE;

}

void lib_config(void) {

}

void lib_init(void) {

  GumDarwinModule *module = NULL;
  gum_darwin_enumerate_modules(mach_task_self(), lib_get_main_module, &module);

  FVERBOSE("Sections:");
  gum_darwin_module_enumerate_sections(module, lib_get_text_section, NULL);

}

guint64 lib_get_text_base(void) {

  if (text_base == 0) FFATAL("Lib not initialized");
  return text_base;

}

guint64 lib_get_text_limit(void) {

  if (text_limit == 0) FFATAL("Lib not initialized");
  return text_limit;

}

#endif

