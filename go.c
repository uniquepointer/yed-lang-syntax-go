#include <yed/plugin.h>
#include <yed/syntax.h>

static yed_syntax syn;


#define _CHECK(x, r)                                                      \
    do {                                                                      \
        if (x) {                                                              \
            LOG_FN_ENTER();                                                   \
            yed_log("[!] " __FILE__ ":%d regex error for '%s': %s", __LINE__, \
                    r,                                                        \
                    yed_syntax_get_regex_err(&syn));                          \
            LOG_EXIT();                                                       \
        }                                                                     \
    } while (0)

#define SYN()          yed_syntax_start(&syn)
#define ENDSYN()       yed_syntax_end(&syn)
#define APUSH(s)       yed_syntax_attr_push(&syn, s)
#define APOP(s)        yed_syntax_attr_pop(&syn)
#define RANGE(r)       _CHECK(yed_syntax_range_start(&syn, r), r)
#define ONELINE()      yed_syntax_range_one_line(&syn)
#define SKIP(r)        _CHECK(yed_syntax_range_skip(&syn, r), r)
#define ENDRANGE(r)    _CHECK(yed_syntax_range_end(&syn, r), r)
#define REGEX(r)       _CHECK(yed_syntax_regex(&syn, r), r)
#define REGEXSUB(r, g) _CHECK(yed_syntax_regex_sub(&syn, r, g), r)
#define KWD(k)         yed_syntax_kwd(&syn, k)

#ifdef __APPLE__
    #define WB "[[:>:]]"
#else
    #define WB "\\b"
#endif

void
estyle(yed_event* event)
{
    yed_syntax_style_event(&syn, event);
}
void
ebuffdel(yed_event* event)
{
    yed_syntax_buffer_delete_event(&syn, event);
}
void
ebuffmod(yed_event* event)
{
    yed_syntax_buffer_mod_event(&syn, event);
}
void
eline(yed_event* event)
{
    yed_frame* frame;

    frame = event->frame;

    if (!frame
            ||  !frame->buffer
            ||  frame->buffer->kind != BUFF_KIND_FILE
            ||  frame->buffer->ft != yed_get_ft("Golang"))
    {
        return;
    }

    yed_syntax_line_event(&syn, event);
}


void
unload(yed_plugin* self)
{
    yed_syntax_free(&syn);
    ys->redraw = 1;
}

int
yed_plugin_boot(yed_plugin* self)
{
    yed_event_handler style;
    yed_event_handler buffdel;
    yed_event_handler buffmod;
    yed_event_handler line;


    YED_PLUG_VERSION_CHECK();

    yed_plugin_set_unload_fn(self, unload);

    style.kind = EVENT_STYLE_CHANGE;
    style.fn   = estyle;
    yed_plugin_add_event_handler(self, style);

    buffdel.kind = EVENT_BUFFER_PRE_DELETE;
    buffdel.fn   = ebuffdel;
    yed_plugin_add_event_handler(self, buffdel);

    buffmod.kind = EVENT_BUFFER_POST_MOD;
    buffmod.fn   = ebuffmod;
    yed_plugin_add_event_handler(self, buffmod);

    line.kind = EVENT_LINE_PRE_DRAW;
    line.fn   = eline;
    yed_plugin_add_event_handler(self, line);


    SYN();
    APUSH("&code-comment");
    RANGE("/\\*");
    ENDRANGE("\\*/");
    RANGE("//");
    ONELINE();
    ENDRANGE("$");
    APOP();

    APUSH("&code-string");
    REGEX("'(\\\\.|[^'\\\\])'");

    RANGE("\"");
    ONELINE();
    SKIP("\\\\\"");
    APUSH("&code-escape");
    REGEX("\\\\.");
    APOP();
    ENDRANGE("\"");
    APOP();

    APUSH("&code-fn-call");
    REGEXSUB("([[:alpha:]_][[:alnum:]_]*)[[:space:]]*\\(", 1);
    APOP();

    APUSH("&code-number");
    REGEXSUB("(^|[^[:alnum:]_])(-?([[:digit:]]+\\.[[:digit:]]*)|(([[:digit:]]*\\.[[:digit:]]+))(e\\+[[:digit:]]+)?[fFlL]?)"WB,
             2);
    REGEXSUB("(^|[^[:alnum:]_])(-?[[:digit:]]+(([uU]?[lL]{0,2})|([lL]{0,2}[uU]?))?)"WB,
             2);
    REGEXSUB("(^|[^[:alnum:]_])(0[xX][0-9a-fA-F]+(([uU]?[lL]{0,2})|([lL]{0,2}[uU]?))?)"WB,
             2);
    APOP();

    APUSH("&code-keyword");
    KWD("func");
    KWD("interface");
    KWD("package");
    KWD("const");
    KWD("import");
    KWD("var");
    APOP();

    APUSH("&code-control-flow");
    KWD("break");
    KWD("case");
    KWD("continue");
    KWD("default");
    KWD("fallthrough");
    KWD("else");
    KWD("for");
    KWD("goto");
    KWD("if");
    KWD("return");
    KWD("switch");
    KWD("range");
    KWD("select");
    KWD("defer");
    REGEXSUB("^[[:space:]]*([[:alpha:]_][[:alnum:]_]*):", 1);
    APOP();

    APUSH("&code-typename");
    KWD("int");
    KWD("int8");
    KWD("int16");
    KWD("int32");
    KWD("int64");
    KWD("uint");
    KWD("uint8");
    KWD("uint16");
    KWD("uint32");
    KWD("uint64");
    KWD("uintptr");
    KWD("float");
    KWD("float32");
    KWD("complex64");
    KWD("complex128");
    KWD("string");
    KWD("byte");
    KWD("rune");
    KWD("bool");
    KWD("map");
    KWD("struct");
    KWD("type");
    APOP();

    APUSH("&code-preprocessor");
    KWD("import");
    KWD("package");
    KWD("go");
    KWD("make");
    KWD("chan");
    APOP();

    APUSH("&code-constant");
    KWD("nil");
    KWD("stdin");
    KWD("stdout");
    KWD("stderr");
    APOP();

    APUSH("&code-field");
    REGEXSUB("(\\.)[[:space:]]*([[:alpha:]_][[:alnum:]_]*)", 2);
    APOP();
    ENDSYN();

    ys->redraw = 1;

    return 0;
}
