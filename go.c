#include <yed/plugin.h>
#include <yed/highlight.h>

#define ARRAY_LOOP(a) for (__typeof((a)[0]) *it = (a); it < (a) + (sizeof(a) / sizeof((a)[0])); ++it)

highlight_info hinfo;


void
unload(yed_plugin* self);
void
syntax_go_line_handler(yed_event* event);
void
syntax_go_frame_handler(yed_event* event);
void
syntax_go_buff_mod_pre_handler(yed_event* event);
void
syntax_go_buff_mod_post_handler(yed_event* event);
void
syntax_go_fmt(yed_event* event);
void
syntax_go_reload_fmt(yed_event* event);
void
buff_path_for_fmt(yed_event* event);

char bufferLoc[512];

int
yed_plugin_boot(yed_plugin* self)
{
    yed_event_handler frame, line, buff_mod_pre, buff_mod_post;
    yed_event_handler gofmt;
    yed_event_handler reload;
    yed_event_handler getpath;

    char*             kwds[] = {
        "func",
        "interface",
        "package",
        "const",
        "import",
        "var",

    };
    char* pp_kwds[] = {
        "import",
        "package",
        "defer",
        "go",
        "make",
        "chan",
    };
    char* control_flow[] = {
        "break",
        "case",
        "continue",
        "default",
        "fallthrough",
        "else",
        "for",
        "goto",
        "if",
        "return",
        "switch",
        "range",
        "select",
    };
    char* typenames[] = {
        "int",     "int8",      "int16",      "int32",  "int64",   "uint",
        "uint8",   "uint16",    "uint32",     "uint64", "uintptr", "float",
        "float32", "complex64", "complex128", "string", "byte",    "rune",
        "bool",    "map",       "struct",     "type",
    };

    YED_PLUG_VERSION_CHECK();

    yed_plugin_set_unload_fn(self, unload);

    frame.kind         = EVENT_FRAME_PRE_BUFF_DRAW;
    frame.fn           = syntax_go_frame_handler;
    line.kind          = EVENT_LINE_PRE_DRAW;
    line.fn            = syntax_go_line_handler;
    buff_mod_pre.kind  = EVENT_BUFFER_PRE_MOD;
    buff_mod_pre.fn    = syntax_go_buff_mod_pre_handler;
    buff_mod_post.kind = EVENT_BUFFER_POST_MOD;
    buff_mod_post.fn   = syntax_go_buff_mod_post_handler;

    getpath.kind       = EVENT_BUFFER_POST_LOAD;
    getpath.fn         = buff_path_for_fmt;
    gofmt.kind         = EVENT_BUFFER_POST_WRITE;
    gofmt.fn           = syntax_go_fmt;
    yed_plugin_add_event_handler(self, frame);
    yed_plugin_add_event_handler(self, line);
    yed_plugin_add_event_handler(self, buff_mod_pre);
    yed_plugin_add_event_handler(self, buff_mod_post);
    yed_plugin_add_event_handler(self, getpath);
    yed_plugin_add_event_handler(self, gofmt);

    highlight_info_make(&hinfo);

    ARRAY_LOOP(kwds)
        highlight_add_kwd(&hinfo, *it, HL_KEY);
    ARRAY_LOOP(pp_kwds)
        highlight_add_prefixed_kwd(&hinfo, '#', *it, HL_PP);
    ARRAY_LOOP(control_flow)
        highlight_add_kwd(&hinfo, *it, HL_CF);
    ARRAY_LOOP(typenames)
        highlight_add_kwd(&hinfo, *it, HL_TYPE);
    highlight_add_kwd(&hinfo, "__VA_ARGS__", HL_PP);
    highlight_add_kwd(&hinfo, "__FILE__", HL_PP);
    highlight_add_kwd(&hinfo, "__func__", HL_PP);
    highlight_add_kwd(&hinfo, "__FUNCTION__", HL_PP);
    highlight_add_kwd(&hinfo, "__LINE__", HL_PP);
    highlight_add_kwd(&hinfo, "__DATE__", HL_PP);
    highlight_add_kwd(&hinfo, "__TIME__", HL_PP);
    highlight_add_kwd(&hinfo, "__STDC__", HL_PP);
    highlight_add_kwd(&hinfo, "__STDC_VERSION__", HL_PP);
    highlight_add_kwd(&hinfo, "__STDC_HOSTED__", HL_PP);
    highlight_add_kwd(&hinfo, "__cplusplus", HL_PP);
    highlight_add_kwd(&hinfo, "__OBJC__", HL_PP);
    highlight_add_kwd(&hinfo, "__ASSEMBLER__", HL_PP);
    highlight_add_kwd(&hinfo, "NULL", HL_CON);
    highlight_add_kwd(&hinfo, "stdin", HL_CON);
    highlight_add_kwd(&hinfo, "stdout", HL_CON);
    highlight_add_kwd(&hinfo, "stderr", HL_CON);
    highlight_suffixed_words(&hinfo, '(', HL_CALL);
    highlight_numbers(&hinfo);
    highlight_within(&hinfo, "\"", "\"", '\\', -1, HL_STR);
    highlight_within(&hinfo, "'", "'", '\\', 1, HL_CHAR);
    highlight_to_eol_from(&hinfo, "//", HL_COMMENT);
    highlight_within_multiline(&hinfo, "/*", "*/", 0, HL_COMMENT);
    highlight_within_multiline(&hinfo, "#if 0", "#endif", 0, HL_COMMENT);

    ys->redraw = 1;

    return 0;
}

void
unload(yed_plugin* self)
{
    highlight_info_free(&hinfo);
    ys->redraw = 1;
}

void
syntax_go_frame_handler(yed_event* event)
{
    yed_frame* frame;

    frame = event->frame;

    if (!frame || !frame->buffer || frame->buffer->kind != BUFF_KIND_FILE ||
        frame->buffer->ft != yed_get_ft("Golang"))
    {
        return;
    }

    highlight_frame_pre_draw_update(&hinfo, event);
}

void
syntax_go_line_handler(yed_event* event)
{
    yed_frame* frame;

    frame = event->frame;

    if (!frame || !frame->buffer || frame->buffer->kind != BUFF_KIND_FILE ||
        frame->buffer->ft != yed_get_ft("Golang"))
    {
        return;
    }

    highlight_line(&hinfo, event);
}

void
syntax_go_buff_mod_pre_handler(yed_event* event)
{
    if (event->buffer == NULL || event->buffer->kind != BUFF_KIND_FILE ||
        event->buffer->ft != yed_get_ft("Golang"))
    {
        return;
    }

    highlight_buffer_pre_mod_update(&hinfo, event);
}

void
syntax_go_buff_mod_post_handler(yed_event* event)
{
    if (event->buffer == NULL || event->buffer->kind != BUFF_KIND_FILE ||
        event->buffer->ft != yed_get_ft("Golang"))
    {
        return;
    }

    highlight_buffer_post_mod_update(&hinfo, event);
}

void
fmtFile(void)
{
    int  output_len, status;

    char cmd_buff[1024];

    snprintf(cmd_buff, sizeof(cmd_buff), "gofmt -w %s", bufferLoc);

    yed_run_subproc(cmd_buff, &output_len, &status);

    if (status != 0)
    {
        yed_cprint("Failure to format golang\n");
    }
    else
    {
        yed_cprint("Formatted buffer\n");
    }
}

void
syntax_go_fmt(yed_event* event)
{
    fmtFile();
    YEXE("buffer-reload");
    yed_cprint("Buffer reloaded");
}

void
buff_path_for_fmt(yed_event* event)
{
    yed_buffer* buffer;
    yed_frame*  frame;

    if (!ys->active_frame)
    {
        yed_cerr("no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer)
    {
        yed_cerr("active frame has no buffer");
        return;
    }

    buffer = frame->buffer;

    if (buffer->name)
    {
        strcpy(bufferLoc, buffer->path);
    }
    else
    {
        yed_cerr("buffer has no path");
    }
}
