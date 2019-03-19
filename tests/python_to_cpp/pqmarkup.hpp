namespace syntax_highlighter_for_pqmarkup {
#include "syntax_highlighter_for_pqmarkup.hpp"
}

Array<String> argv;

class Exception
{
public:
    String message;
    int line;
    int column;
    int pos;
    template <typename T1, typename T2, typename T3, typename T4> Exception(const T1 &message, const T2 &line, const T3 &column, const T4 &pos)
    {
        this->message = message;
        this->line = line;
        this->column = column;
        this->pos = pos;
    }
};

class Converter
{
public:
    Array<int> to_html_called_inside_to_html_outer_pos_list;
    bool habr_html;
    bool ohd;
    decltype(false) highlight_style_was_added = false;
    String instr;

    template <typename T1, typename T2> Converter(const T1 &habr_html, const T2 &ohd)
    {
        this->habr_html = habr_html;
        this->ohd = ohd;
    }

    template <typename T1> auto instr_pos_to_line_column(T1 pos)
    {
        pos += sum(to_html_called_inside_to_html_outer_pos_list);
        auto line = 1;
        auto line_start = -1;
        auto t = 0;
        while (t < pos) {
            if (instr[t] == u'\r') {
                if (t < pos - 1 && instr[t + 1] == u'\n')
                    t++;
                line++;
                line_start = t;
            }
            else if (instr[t] == u'\n') {
                line++;
                line_start = t;
            }
            t++;
        }
        return make_tuple(line, pos - line_start, pos);
    }

    template <typename T1, typename T3 = decltype(0)> String to_html(const T1 &instr, File* const outfilef = nullptr, const T3 &outer_pos = 0)
    {
        to_html_called_inside_to_html_outer_pos_list.append(outer_pos);
        Array<String> result;
        class Writer
        {
        public:
            std::function<void(const String&)> write;
        };
        auto outfile = Writer();
        if (outfilef == nullptr)
            outfile.write = [&result](const auto &s){return result.append(s);};
        else
            outfile.write = [&outfilef](const auto &s){return outfilef->write(s);};
        if (to_html_called_inside_to_html_outer_pos_list.len() == 1)
            this->instr = instr;

        auto exit_with_error = [this](const auto &message, const auto &pos)
        {
            auto p = instr_pos_to_line_column(pos);
            throw Exception(message, _get<0>(p), _get<1>(p), _get<2>(p));
        };
        auto i = 0;
        auto next_char = [&i, &instr](const decltype(1) offset = 1)
        {
            return i + offset < instr.len() ? instr[i + offset] : u'\0'_C;
        };

        auto i_next_str = [&i, &instr](const auto &str)
        {
            return instr[range_el(i + 1, i + 1 + str.len())] == str;
        };

        auto prev_char = [&i, &instr](const decltype(1) offset = 1)
        {
            return i - offset >= 0 ? instr[i - offset] : u'\0'_C;
        };

        auto html_escape = [this](auto str)
        {
            str = str.replace(u"&"_S, u"&amp;"_S).replace(u"<"_S, u"&lt;"_S);
            if (habr_html)
                str = str.replace(u"\""_S, u"&quot;"_S);
            return str;
        };
        auto html_escapeq = [](const auto &str)
        {
            return str.replace(u"&"_S, u"&amp;"_S).replace(u"\""_S, u"&quot;"_S);
        };
        auto writepos = 0;
        auto write_to_pos = [&html_escape, &instr, &outfile, &writepos](const auto &pos, const auto &npos)
        {
            outfile.write(html_escape(instr[range_el(writepos, pos)]));
            writepos = npos;
        };

        auto write_to_i = [&i, &outfile, &write_to_pos](const auto &add_str, const decltype(1) skip_chars = 1)
        {
            write_to_pos(i, i + skip_chars);
            outfile.write(add_str);
        };

        auto find_ending_pair_quote = [&exit_with_error, &instr](auto i)
        {
            assert(instr[i] == u'‘');
            auto startqpos = i;
            auto nesting_level = 0;
            while (true) {
                if (i == instr.len())
                    exit_with_error(u"Unpaired left single quotation mark"_S, startqpos);
                switch (instr[i])
                {
                case u'‘':
                    nesting_level++;
                    break;
                case u'’':
                    if (--nesting_level == 0)
                        return i;
                    break;
                }
                i++;
            }
        };

        auto find_ending_sq_bracket = [&exit_with_error](const auto &str, auto i, const decltype(0) start = 0)
        {
            auto starti = i;
            assert(str[i] == u'[');
            auto nesting_level = 0;
            while (true) {
                switch (str[i])
                {
                case u'[':
                    nesting_level++;
                    break;
                case u']':
                    if (--nesting_level == 0)
                        return i;
                    break;
                }
                i++;
                if (i == str.len())
                    exit_with_error(u"Unended comment started"_S, start + starti);
            }
        };

        auto remove_comments = [&find_ending_sq_bracket](auto s, auto start, const decltype(3) level = 3)
        {
            while (true) {
                auto j = s.findi(u"["_S * level);
                if (j == -1)
                    break;
                auto k = find_ending_sq_bracket(s, j, start) + 1;
                start += k - j;
                s = s[range_el(0, j)] + s[range_ei(k)];
            }
            return s;
        };
        Dict<int, String> nonunique_links;
        auto link = u""_S;

        auto write_http_link = [&exit_with_error, &find_ending_pair_quote, &find_ending_sq_bracket, &html_escape, &html_escapeq, &i, &instr, &link, &next_char, &nonunique_links, &outfile, &remove_comments, &write_to_pos](const auto &startpos, const int endpos, const decltype(1) q_offset = 1, decltype(u""_S) text = u""_S)
        {
            auto nesting_level = 0;
            i += 2;
            while (true) {
                if (i == instr.len())
                    exit_with_error(u"Unended link"_S, endpos + q_offset);
                switch (instr[i])
                {
                case u'[':
                    nesting_level++;
                    break;
                case u']':
                    if (nesting_level == 0)
                        goto break_;
                    nesting_level--;
                    break;
                case u' ':
                    goto break_;
                    break;
                }
                i++;
            }
            break_:;
            link = html_escapeq(instr[range_el(endpos + 1 + q_offset, i)]);
            String tag = u"<a href=\""_S + link + u"\""_S;
            if (link.starts_with(u"./"_S))
                tag += u" target=\"_self\""_S;

            if (instr[i] == u' ') {
                tag += u" title=\""_S;
                if (next_char() == u'‘') {
                    auto endqpos2 = find_ending_pair_quote(i + 1);
                    if (instr[endqpos2 + 1] != u']')
                        exit_with_error(u"Expected `]` after `’`"_S, endqpos2 + 1);
                    tag += html_escapeq(remove_comments(instr[range_el(i + 2, endqpos2)], i + 2));
                    i = endqpos2 + 1;
                }
                else {
                    auto endb = find_ending_sq_bracket(instr, endpos + q_offset);
                    tag += html_escapeq(remove_comments(instr[range_el(i + 1, endb)], i + 1));
                    i = endb;
                }
                tag += u"\""_S;
            }
            if (next_char() == u'[' && next_char(2) == u'-') {
                auto j = i + 3;
                while (j < instr.len()) {
                    if (instr[j] == u']') {
                        nonunique_links.set(to_int(instr[range_el(i + 3, j)]), link);
                        i = j;
                        break;
                    }
                    if (!instr[j].is_digit())
                        break;
                    j++;
                }
            }
            if (text == u"") {
                write_to_pos(startpos, i + 1);
                text = html_escape(remove_comments(instr[range_el(startpos + q_offset, endpos)], startpos + q_offset));
            }
            outfile.write(tag + u">"_S + (text != u"" ? text : link) + u"</a>"_S);
        };

        auto write_note = [&exit_with_error, &find_ending_pair_quote, &html_escape, &html_escapeq, &i, &instr, &outfile, &remove_comments, &write_to_pos](const auto &startpos, const auto &endpos, const decltype(1) q_offset = 1)
        {
            i += q_offset;
            auto endqpos2 = find_ending_pair_quote(i + 1);
            if (instr[endqpos2 + 1] != u']')
                exit_with_error(u"Bracket ] should follow after ’"_S, endqpos2 + 1);
            write_to_pos(startpos, endqpos2 + 2);
            outfile.write(u"<abbr title=\""_S + html_escapeq(remove_comments(instr[range_el(i + 2, endqpos2)], i + 2)) + u"\">"_S + html_escape(remove_comments(instr[range_el(startpos + q_offset, endpos)], startpos + q_offset)) + u"</abbr>"_S);
            i = endqpos2 + 1;
        };
        auto endi = 0;
        auto numbered_link = [&endi, &exit_with_error, &i, &instr, &link, &next_char, &nonunique_links](const decltype(1) offset = 1)
        {
            if (next_char(offset) == u'-' && next_char(offset + 1).is_digit()) {
                auto j = i + offset + 1;
                while (j < instr.len()) {
                    if (instr[j] == u']') {
                        try
                        {
                            link = nonunique_links[to_int(instr[range_el(i + offset + 1, j)])];
                        }
                        catch (const KeyError&)
                        {
                            exit_with_error(u"Link with such index was not declared previously"_S, i + offset + 1);
                        }
                        endi = j;
                        return true;
                    }
                    if (!instr[j].is_digit())
                        break;
                    j++;
                }
            }
            return false;
        };
        auto ordered_list_current_number = -1;
        auto close_ordered_list = [&ordered_list_current_number, &write_to_i]()
        {
            if (ordered_list_current_number != -1) {
                write_to_i(u"</li>\n</ol>\n"_S, 0);
                ordered_list_current_number = -1;
            }
        };
        auto in_unordered_list = false;
        auto close_unordered_list = [&in_unordered_list, &write_to_i]()
        {
            if (in_unordered_list) {
                write_to_i(u"</li>\n</ul>\n"_S, 0);
                in_unordered_list = false;
            }
        };
        Array<String> ending_tags;
        auto new_line_tag = u"\0"_S;

        while (i < instr.len()) {
            auto ch = instr[i];
            if (i == 0 || prev_char() == u'\n') {
                if (ch == u'.' && (in(next_char(), u" ‘"_S))) {
                    close_ordered_list();
                    auto s = u""_S;
                    if (!in_unordered_list) {
                        s = u"<ul>\n<li>"_S;
                        in_unordered_list = true;
                    }
                    else
                        s = u"</li>\n<li>"_S;
                    write_to_i(s);
                    new_line_tag = u""_S;
                    if (next_char() == u' ')
                        i++;
                    else {
                        auto endqpos = find_ending_pair_quote(i + 1);
                        outfile.write(to_html(instr[range_el(i + 2, endqpos)], nullptr, i + 2));
                        i = endqpos;
                    }
                    writepos = i + 1;
                }
                else {
                    close_unordered_list();
                    if (ch.is_digit()) {
                        auto j = i + 1;
                        while (j < instr.len()) {
                            if (!instr[j].is_digit())
                                break;
                            j++;
                        }
                        if (instr[range_el(j, j + 1)] == u'.' && in(instr[range_el(j + 1, j + 2)], make_tuple(u" "_S, u"‘"_S))) {
                            auto value = to_int(instr[range_el(i, j)]);
                            auto s = u""_S;
                            if (ordered_list_current_number == -1) {
                                s = (value == 1 ? u"<ol>"_S : u"<ol start=\""_S + String(value) + u"\">"_S) + u"\n<li>"_S;
                                ordered_list_current_number = value;
                            }
                            else {
                                s = u"</li>\n"_S + (value == ordered_list_current_number + 1 ? u"<li>"_S : u"<li value=\""_S + String(value) + u"\">"_S);
                                ordered_list_current_number = value;
                            }
                            write_to_i(s);
                            new_line_tag = u""_S;
                            if (instr[j + 1] == u' ')
                                i = j + 1;
                            else {
                                auto endqpos = find_ending_pair_quote(j + 1);
                                outfile.write(to_html(instr[range_el(j + 2, endqpos)], nullptr, j + 2));
                                i = endqpos;
                            }
                            writepos = i + 1;
                        }
                        else
                            close_ordered_list();
                    }
                    else
                        close_ordered_list();
                }
                if (ch == u' ')
                    write_to_i(u"&emsp;"_S);
                else if (ch == u'-') {
                    if (i_next_str(u"--"_S)) {
                        auto j = i + 3;
                        while (true) {
                            if (j == instr.len() || instr[j] == u'\n') {
                                write_to_i(u"<hr />\n"_S);
                                if (j == instr.len())
                                    j--;
                                i = j;
                                writepos = j + 1;
                                break;
                            }
                            if (instr[j] != u'-')
                                break;
                            j++;
                        }
                    }
                }
                else if (in(ch, make_tuple(u">"_S, u"<"_S)) && (in(next_char(), u" ‘["_S))) {
                    write_to_pos(i, i + 2);
                    outfile.write(u"<blockquote"_S + ((ch == u'<') * u" class=\"re\""_S) + u">"_S);
                    if (next_char() == u' ')
                        new_line_tag = u"</blockquote>"_S;
                    else {
                        if (next_char() == u'[') {
                            if (numbered_link(2)) {
                                auto linkstr = link;
                                if (linkstr.len() > 57)
                                    linkstr = linkstr[range_el(0, linkstr.rfindi(u"/"_S, 0, 47) + 1)] + u"..."_S;
                                outfile.write(u"<a href=\""_S + link + u"\">["_S + instr[range_el(i + 3, endi)] + u"]<i>"_S + linkstr + u"</i></a>"_S);
                                i = endi + 1;
                            }
                            else {
                                i++;
                                auto endb = find_ending_sq_bracket(instr, i);
                                auto linkn = u""_S;
                                if (instr[range_el(endb + 1, endb + 3)] == u"[-")
                                    linkn = u"["_S + instr[range_el(endb + 3, find_ending_sq_bracket(instr, endb + 1))] + u"]"_S;
                                link = instr[range_el(i + 1, endb)];
                                auto spacepos = link.findi(u" "_S);
                                if (spacepos != -1)
                                    link = link[range_el(0, spacepos)];
                                if (link.len() > 57)
                                    link = link[range_el(0, link.rfindi(u"/"_S, 0, 47) + 1)] + u"..."_S;
                                write_http_link(i, i, 0, linkn + u"<i>"_S + link + u"</i>"_S);
                                i++;
                            }
                            if (instr[range_el(i, i + 2)] != u":‘")
                                exit_with_error(u"Quotation with url should always has :‘...’ after ["_S + link[range_el(0, link.findi(u":"_S))] + u"://url]"_S, i);
                            outfile.write(u":<br />\n"_S);
                            writepos = i + 2;
                        }
                        else {
                            auto endqpos = find_ending_pair_quote(i + 1);
                            if (instr[range_el(endqpos + 1, endqpos + 2)] == u'[') {
                                auto startqpos = i + 1;
                                i = endqpos;
                                outfile.write(u"<i>"_S);
                                assert(writepos == startqpos + 1);
                                writepos = startqpos;
                                write_http_link(startqpos, endqpos);
                                outfile.write(u"</i>"_S);
                                i++;
                                if (instr[range_el(i, i + 2)] != u":‘")
                                    exit_with_error(u"Quotation with url should always has :‘...’ after ["_S + link[range_el(0, link.findi(u":"_S))] + u"://url]"_S, i);
                                outfile.write(u":<br />\n"_S);
                                writepos = i + 2;
                            }
                            else if (instr[range_el(endqpos + 1, endqpos + 2)] == u':') {
                                outfile.write(u"<i>"_S + instr[range_el(i + 2, endqpos)] + u"</i>:<br />\n"_S);
                                i = endqpos + 1;
                                if (instr[range_el(i, i + 2)] != u":‘")
                                    exit_with_error(u"Quotation with author's name should be in the form >‘Author's name’:‘Quoted text.’"_S, i);
                                writepos = i + 2;
                            }
                        }
                        ending_tags.append(u"</blockquote>"_S);
                    }
                    i++;
                }
            }

            if (ch == u'‘') {
                auto prevci = i - 1;
                auto prevc = prevci >= 0 ? instr[prevci] : u'\0'_C;
                auto startqpos = i;
                i = find_ending_pair_quote(i);
                auto endqpos = i;
                auto str_in_b = u""_S;
                if (prevc == u')') {
                    auto openb = instr.rfindi(u"("_S, 0, prevci - 1);
                    if (openb != -1 && openb > 0) {
                        str_in_b = instr[range_el(openb + 1, startqpos - 1)];
                        prevci = openb - 1;
                        prevc = instr[prevci];
                    }
                }
                if (in(prevc, u"PР"_S)) {
                    write_to_pos(prevci, endqpos + 1);
                    auto title = u""_S;
                    int endqpos2;
                    if (i_next_str(u"[‘"_S)) {
                        endqpos2 = find_ending_pair_quote(i + 2);
                        if (instr[endqpos2 + 1] != u']')
                            exit_with_error(u"Expected `]` after `’`"_S, endqpos2 + 1);
                        title = u" title=\""_S + html_escapeq(remove_comments(instr[range_el(i + 3, endqpos2)], i + 3)) + u"\""_S;
                    }
                    auto imgtag = u"<img"_S;
                    if (str_in_b != u"") {
                        auto wh = str_in_b.replace(u","_S, u" "_S).split(u" "_S);
                        assert(in(wh.len(), make_tuple(1, 2)));
                        imgtag += u" width=\""_S + _get<0>(wh) + u"\" height=\""_S + wh.last() + u"\""_S;
                    }
                    imgtag += u" src=\""_S + instr[range_el(startqpos + 1, endqpos)] + u"\""_S + title + u" />"_S;
                    if (i_next_str(u"[http"_S) || i_next_str(u"[./"_S)) {
                        write_http_link(startqpos, endqpos, 1, imgtag);
                        writepos = i + 1;
                    }
                    else if (i_next_str(u"[‘"_S)) {
                        outfile.write(imgtag);
                        writepos = endqpos2 + 2;
                        i = endqpos2 + 1;
                    }
                    else {
                        outfile.write(imgtag);
                        i = endqpos;
                    }
                }
                else if (i_next_str(u"[http"_S) || i_next_str(u"[./"_S))
                    write_http_link(startqpos, endqpos);
                else if (next_char() == u'[' && numbered_link(2)) {
                    i = endi;
                    write_to_pos(startqpos, i + 1);
                    outfile.write(u"<a href=\""_S + link + u"\">"_S + html_escape(instr[range_el(startqpos + 1, endqpos)]) + u"</a>"_S);
                }
                else if (i_next_str(u"[‘"_S))
                    write_note(startqpos, endqpos);
                else if (next_char() == u'{' && (habr_html || ohd)) {
                    auto nesting_level = 0;
                    i += 2;
                    while (true) {
                        if (i == instr.len())
                            exit_with_error(u"Unended spoiler"_S, endqpos + 1);
                        switch (instr[i])
                        {
                        case u'{':
                            nesting_level++;
                            break;
                        case u'}':
                            if (nesting_level == 0)
                                goto break_1;
                            nesting_level--;
                            break;
                        }
                        i++;
                    }
                    break_1:;
                    write_to_pos(prevci + 1, i + 1);
                    auto outer_p = endqpos + (instr[endqpos + 2] == u'\n' ? 3 : 2);
                    if (habr_html)
                        outfile.write(u"<spoiler title=\""_S + remove_comments(instr[range_el(startqpos + 1, endqpos)], startqpos + 1).replace(u"\""_S, u"''"_S) + u"\">\n"_S + (to_html(instr[range_el(outer_p, i)], nullptr, outer_p)) + u"</spoiler>\n"_S);
                    else
                        outfile.write(u"<span class=\"spoiler_title\" onclick=\"return spoiler2(this, event)\">"_S + remove_comments(instr[range_el(startqpos + 1, endqpos)], startqpos + 1) + u"<br /></span>"_S + u"<div class=\"spoiler_text\" style=\"display: none\">\n"_S + (to_html(instr[range_el(outer_p, i)], nullptr, outer_p)) + u"</div>\n"_S);
                    if ((next_char() == u'\n' && !in_unordered_list && ordered_list_current_number == -1)) {
                        i++;
                        writepos = i + 1;
                    }
                }
                else if (prevc == u'\'') {
                    auto t = startqpos - 1;
                    while (t >= 0) {
                        if (instr[t] != u'\'')
                            break;
                        t--;
                    }
                    auto eat_left = startqpos - 1 - t;
                    t = endqpos + 1;
                    while (t < instr.len()) {
                        if (instr[t] != u'\'')
                            break;
                        t++;
                    }
                    auto eat_right = t - (endqpos + 1);
                    write_to_pos(startqpos - eat_left, t);
                    outfile.write(instr[range_el(startqpos + eat_left, endqpos - eat_right + 1)]);
                }
                else if (in(prevc, u"0OО"_S)) {
                    write_to_pos(prevci, endqpos + 1);
                    outfile.write(html_escape(instr[range_el(startqpos + 1, endqpos)]).replace(u"\n"_S, u"<br />\n"_S));
                }
                else if (prevc == u'#') {
                    auto ins = instr[range_el(startqpos + 1, endqpos)];
                    write_to_pos(prevci, endqpos + 1);
                    if (habr_html) {
                        auto contains_new_line = in(u'\n'_C, ins);
                        outfile.write((str_in_b != u"" ? u"<source lang=\""_S + str_in_b + u"\">"_S : contains_new_line ? u"<source>"_S : u"<code>"_S) + ins + (str_in_b != u"" || contains_new_line ? u"</source>"_S : u"</code>"_S));
                    }
                    else {
                        auto pre = u"<pre "_S + (_get<0>(ins) == u'\n' ? u"class=\"code_block\""_S : u"style=\"display: inline\""_S) + u">"_S;
                        if (ohd && syntax_highlighter_for_pqmarkup::is_lang_supported(str_in_b)) {
                            if (!(highlight_style_was_added)) {
                                outfile.write(syntax_highlighter_for_pqmarkup::css);
                                highlight_style_was_added = true;
                            }
                            try
                            {
                                outfile.write(pre + syntax_highlighter_for_pqmarkup::highlight(str_in_b, ins) + u"</pre>"_S);
                            }
                            catch (const syntax_highlighter_for_pqmarkup::Error& e)
                            {
                                exit_with_error(u"Syntax highlighter: "_S + e.message, startqpos + 1 + e.pos);
                            }
                        }
                        else
                            outfile.write(pre + html_escape(ins) + u"</pre>"_S);
                    }
                    if (_get<0>(ins) == u'\n' && instr[range_el(i + 1, i + 2)] == u'\n') {
                        outfile.write(u"\n"_S);
                        new_line_tag = u""_S;
                    }
                }
                else if (in(prevc, u"TТ"_S)) {
                    write_to_pos(prevci, endqpos + 1);
                    auto header_row = false;
                    auto hor_row_align = u""_S;
                    auto ver_row_align = u""_S;

                    class TableCell
                    {
                    public:
                        String text;
                        String attrs;
                        TableCell(const String &text, const String &attrs)
                        {
                            this->text = text;
                            this->attrs = attrs;
                        }
                    };
                    Array<Array<TableCell>> table;
                    auto j = startqpos + 1;
                    while (j < endqpos) {
                        ch = instr[j];
                        if (ch == u'‘') {
                            Array<TableCell> empty_list;
                            table.append(empty_list);
                            auto endrow = find_ending_pair_quote(j);
                            auto hor_col_align = u""_S;
                            auto ver_col_align = u""_S;
                            j++;
                            while (j < endrow) {
                                ch = instr[j];
                                if (ch == u'‘') {
                                    auto end_of_column = find_ending_pair_quote(j);
                                    auto style = u""_S;
                                    if (hor_row_align != u"" || hor_col_align != u"")
                                        style += u"text-align: "_S + (hor_col_align != u"" ? hor_col_align : hor_row_align);
                                    if (ver_row_align != u"" || ver_col_align != u"") {
                                        if (style != u"")
                                            style += u"; "_S;
                                        style += u"vertical-align: "_S + (ver_col_align != u"" ? ver_col_align : ver_row_align);
                                    }
                                    hor_col_align = u""_S;
                                    ver_col_align = u""_S;
                                    table.last().append(TableCell(to_html(instr[range_el(j + 1, end_of_column)], nullptr, j + 1), (header_row ? u"th"_S : u"td"_S) + (style != u"" ? u" style=\""_S + style + u"\""_S : u""_S)));
                                    j = end_of_column;
                                }
                                else if (in(ch, u"<>"_S) && in(instr[range_el(j + 1, j + 2)], make_tuple(u"<"_S, u">"_S))) {
                                    hor_col_align = [&](const auto &a){return a == u"<<" ? u"left"_S : a == u">>" ? u"right"_S : a == u"><" ? u"center"_S : a == u"<>" ? u"justify"_S : throw KeyError(a);}(instr[range_el(j, j + 2)]);
                                    j++;
                                }
                                else if (in(instr[range_el(j, j + 2)], make_tuple(u"/\\"_S, u"\\/"_S))) {
                                    ver_col_align = instr[range_el(j, j + 2)] == u"/\\" ? u"top"_S : u"bottom"_S;
                                    j++;
                                }
                                else if (ch == u'-') {
                                    if (table.last().empty())
                                        exit_with_error(u"Wrong table column span marker \"-\""_S, j);
                                    table.last().append(TableCell(u""_S, u"-"_S));
                                }
                                else if (ch == u'|') {
                                    if (table.len() == 1)
                                        exit_with_error(u"Wrong table row span marker \"|\""_S, j);
                                    table.last().append(TableCell(u""_S, u"|"_S));
                                }
                                else if (instr[range_el(j, j + 3)] == u"[[[")
                                    j = find_ending_sq_bracket(instr, j);
                                else if (!in(ch, u"  \t\n"_S))
                                    exit_with_error(u"Unknown formatting character inside table row"_S, j);
                                j++;
                            }
                            header_row = false;
                            hor_row_align = u""_S;
                            ver_row_align = u""_S;
                        }
                        else if (in(ch, u"HН"_S))
                            header_row = true;
                        else if (in(ch, u"<>"_S) && in(instr[range_el(j + 1, j + 2)], make_tuple(u"<"_S, u">"_S))) {
                            hor_row_align = [&](const auto &a){return a == u"<<" ? u"left"_S : a == u">>" ? u"right"_S : a == u"><" ? u"center"_S : a == u"<>" ? u"justify"_S : throw KeyError(a);}(instr[range_el(j, j + 2)]);
                            j++;
                        }
                        else if (in(instr[range_el(j, j + 2)], make_tuple(u"/\\"_S, u"\\/"_S))) {
                            ver_row_align = instr[range_el(j, j + 2)] == u"/\\" ? u"top"_S : u"bottom"_S;
                            j++;
                        }
                        else if (instr[range_el(j, j + 3)] == u"[[[")
                            j = find_ending_sq_bracket(instr, j);
                        else if (!in(ch, u"  \t\n"_S))
                            exit_with_error(u"Unknown formatting character inside table"_S, j);
                        j++;
                    }
                    for (auto y : range_el(table.len() - 1, -1).step(-1))
                        for (auto x : range_el(table[y].len() - 1, -1).step(-1))
                            if (in(table[y][x].attrs, make_tuple(u"-"_S, u"|"_S))) {
                                auto xx = x;
                                auto yy = y;
                                while (true)
                                    if (table[yy][xx].attrs == u'-')
                                        xx--;
                                    else if (table[yy][xx].attrs == u'|')
                                        yy--;
                                    else
                                        break;
                                if (xx < x)
                                    table[yy][xx].attrs += u" colspan=\""_S + String(x - xx + 1) + u"\""_S;
                                if (yy < y)
                                    table[yy][xx].attrs += u" rowspan=\""_S + String(y - yy + 1) + u"\""_S;
                                for (auto xxx : range_el(xx, x + 1))
                                    for (auto yyy : range_el(yy, y + 1))
                                        if (make_tuple(xxx, yyy) != make_tuple(xx, yy))
                                            table[yyy][xxx].attrs = u""_S;
                            }
                    auto is_inline = true;
                    if ((prevci == 0 || instr[prevci - 1] == u'\n' || (prevci - 3 >= 0 && instr[range_el(prevci - 3, prevci)] == u"]]]" && instr[range_el(0, 3)] == u"[[[" && find_ending_sq_bracket(instr, 0) == prevci - 1)))
                        is_inline = false;
                    outfile.write(u"<table"_S + (u" style=\"display: inline\""_S * is_inline) + u">\n"_S);
                    for (auto &&row : table) {
                        outfile.write(u"<tr>"_S);
                        for (auto &&cell : row)
                            if (cell.attrs != u"")
                                outfile.write(u"<"_S + cell.attrs + u">"_S + cell.text + u"</"_S + cell.attrs[range_el(0, 2)] + u">"_S);
                        outfile.write(u"</tr>\n"_S);
                    }
                    outfile.write(u"</table>\n"_S);
                    if (!is_inline)
                        new_line_tag = u""_S;
                }
                else if (in(prevc, u"<>"_S) && in(instr[prevci - 1], u"<>"_S)) {
                    write_to_pos(prevci - 1, endqpos + 1);
                    outfile.write(u"<div align=\""_S + ([&](const auto &a){return a == u"<<" ? u"left"_S : a == u">>" ? u"right"_S : a == u"><" ? u"center"_S : a == u"<>" ? u"justify"_S : throw KeyError(a);}(instr[prevci - 1] + prevc)) + u"\">"_S + (to_html(instr[range_el(startqpos + 1, endqpos)], nullptr, startqpos + 1)) + u"</div>\n"_S);
                    new_line_tag = u""_S;
                }
                else if (i_next_str(u":‘"_S) && instr[range_ei(find_ending_pair_quote(i + 2) + 1)][range_el(0, 1)] == u'<') {
                    auto endrq = find_ending_pair_quote(i + 2);
                    i = endrq + 1;
                    write_to_pos(prevci + 1, i + 1);
                    outfile.write(u"<blockquote>"_S + (to_html(instr[range_el(startqpos + 1, endqpos)], nullptr, startqpos + 1)) + u"<br />\n<div align='right'><i>"_S + instr[range_el(endqpos + 3, endrq)] + u"</i></div></blockquote>"_S);
                    new_line_tag = u""_S;
                }
                else {
                    i = startqpos;
                    if (in(prev_char(), u"*_-~"_S)) {
                        write_to_pos(i - 1, i + 1);
                        auto tag = [&](const auto &a){return a == u'*' ? u'b'_C : a == u'_' ? u'u'_C : a == u'-' ? u's'_C : a == u'~' ? u'i'_C : throw KeyError(a);}(prev_char());
                        outfile.write(u"<"_S + tag + u">"_S);
                        ending_tags.append(u"</"_S + tag + u">"_S);
                    }
                    else if (in(prevc, u"HН"_S)) {
                        write_to_pos(prevci, i + 1);
                        auto tag = u"h"_S + String(min(max(3 - (str_in_b == u"" ? 0 : to_int(str_in_b)), 1), 6));
                        outfile.write(u"<"_S + tag + u">"_S);
                        ending_tags.append(u"</"_S + tag + u">"_S);
                    }
                    else if (in(prevc, u"CС"_S)) {
                        write_to_pos(prevci, i + 1);
                        auto which_color = u"color"_S;
                        if (str_in_b[range_el(0, 1)] == u'-') {
                            str_in_b = str_in_b[range_ei(1)];
                            which_color = u"background-color"_S;
                        }
                        if (str_in_b[range_el(0, 1)] == u'#') {
                            auto new_str_in_b = u""_S;
                            for (auto &&c : str_in_b) {
                                auto cc = _get<0>(([&](const auto &a){return a == u'а' ? u"A"_S : a == u'б' ? u"B"_S : a == u'с' ? u"C"_S : a == u'д' ? u"D"_S : a == u'е' ? u"E"_S : a == u'ф' ? u"F"_S : c;}(c.lowercase())));
                                new_str_in_b += c.is_lowercase() ? cc.lowercase() : cc;
                            }
                            str_in_b = new_str_in_b;
                        }
                        else if (in(str_in_b.len(), make_tuple(1, 3)) && str_in_b.is_digit()) {
                            auto new_str = u"#"_S;
                            for (auto &&ii : str_in_b.len() == 3 ? create_array({0, 1, 2}) : create_array({0, 0, 0}))
                                new_str += hex(int((to_int(str_in_b[ii]) * 0x00'FF + 4))/int(8)).zfill(2);
                            str_in_b = new_str;
                        }
                        if (habr_html) {
                            outfile.write(u"<font color=\""_S + str_in_b + u"\">"_S);
                            ending_tags.append(u"</font>"_S);
                        }
                        else {
                            outfile.write(u"<span style=\""_S + which_color + u": "_S + str_in_b + u"\">"_S);
                            ending_tags.append(u"</span>"_S);
                        }
                    }
                    else if (in(make_tuple(instr[range_el(prevci - 1, prevci)], prevc), make_tuple(make_tuple(u"/"_S, u"\\"_S), make_tuple(u"\\"_S, u"/"_S)))) {
                        write_to_pos(prevci - 1, i + 1);
                        auto tag = make_tuple(instr[prevci - 1], prevc) == make_tuple(u"/"_S, u"\\"_S) ? u"sup"_S : u"sub"_S;
                        outfile.write(u"<"_S + tag + u">"_S);
                        ending_tags.append(u"</"_S + tag + u">"_S);
                    }
                    else if (prevc == u'!') {
                        write_to_pos(prevci, i + 1);
                        outfile.write(habr_html ? u"<blockquote>"_S : u"<div class=\"note\">"_S);
                        ending_tags.append(habr_html ? u"</blockquote>"_S : u"</div>"_S);
                    }
                    else
                        ending_tags.append(u"’"_S);
                }
            }
            else if (ch == u'’') {
                write_to_pos(i, i + 1);
                if (ending_tags.empty())
                    exit_with_error(u"Unpaired right single quotation mark"_S, i);
                auto last = ending_tags.pop();
                outfile.write(last);
                if (next_char() == u'\n' && (last.starts_with(u"</h"_S) || in(last, make_tuple(u"</blockquote>"_S, u"</div>"_S)))) {
                    outfile.write(u"\n"_S);
                    i++;
                    writepos++;
                }
            }
            else if (ch == u'`') {
                auto start = i;
                i++;
                while (i < instr.len()) {
                    if (instr[i] != u'`')
                        break;
                    i++;
                }
                auto end = instr.findi((i - start) * u"`"_S, i);
                if (end == -1)
                    exit_with_error(u"Unended ` started"_S, start);
                write_to_pos(start, end + i - start);
                auto ins = instr[range_el(i, end)];
                auto delta = ins.count(u"‘"_S) - ins.count(u"’"_S);
                if (delta > 0)
                    for (auto ii : range_el(0, delta))
                        ending_tags.append(u"’"_S);
                else
                    for (auto ii : range_el(0, -delta))
                        if (ending_tags.pop() != u'’')
                            exit_with_error(u"Unpaired single quotation mark found inside code block/span beginning"_S, start);
                ins = html_escape(ins);
                if (!(in(u'\n'_C, ins)))
                    outfile.write(habr_html ? u"<code>"_S + ins + u"</code>"_S : u"<pre class=\"inline_code\">"_S + ins + u"</pre>"_S);
                else {
                    outfile.write(u"<pre>"_S + ins + u"</pre>"_S + (u"\n"_S * (!(habr_html))));
                    new_line_tag = u""_S;
                }
                i = end + i - start - 1;
            }
            else if (ch == u'[') {
                if (i_next_str(u"http"_S) || i_next_str(u"./"_S) || (i_next_str(u"‘"_S) && !in(prev_char(), u"\r\n\t \0"_S)) || numbered_link()) {
                    auto s = i - 1;
                    while (s >= writepos && !in(instr[s], u"\r\n\t [{(‘“"_S))
                        s--;
                    if (i_next_str(u"‘"_S))
                        write_note(s + 1, i, 0);
                    else if (i_next_str(u"http"_S) || i_next_str(u"./"_S))
                        write_http_link(s + 1, i, 0);
                    else {
                        write_to_pos(s + 1, endi + 1);
                        outfile.write(u"<a href=\""_S + link + u"\">"_S + html_escape(instr[range_el(s + 1, i)]) + u"</a>"_S);
                        i = endi;
                    }
                }
                else if (i_next_str(u"[["_S)) {
                    auto comment_start = i;
                    auto nesting_level = 0;
                    while (true) {
                        switch (instr[i])
                        {
                        case u'[':
                            nesting_level++;
                            break;
                        case u']':
                            if (--nesting_level == 0)
                                goto break_2;
                            break;
                        case u'‘':
                            ending_tags.append(u"’"_S);
                            break;
                        case u'’':
                            assert(ending_tags.pop() == u'’');
                            break;
                        }
                        i++;
                        if (i == instr.len())
                            exit_with_error(u"Unended comment started"_S, comment_start);
                    }
                    break_2:;
                    write_to_pos(comment_start, i + 1);
                    if (instr[range_el(comment_start + 3, comment_start + 4)] != u'[') {
                        outfile.write(u"<!--"_S);
                        outfile.write(remove_comments(instr[range_el(comment_start, i + 1)], comment_start, 4));
                        outfile.write(u"-->"_S);
                    }
                }
                else
                    write_to_i((u"<span class=\"sq\"><span class=\"sq_brackets\">"_S * ohd) + (u"<font color=\"#BFBFBF\">"_S * habr_html) + u"["_S + (u"</font><font color=\"gray\">"_S * habr_html) + (ohd * u"</span>"_S));
            }
            else if (ch == u']')
                write_to_i((u"<span class=\"sq_brackets\">"_S * ohd) + (u"</font><font color=\"#BFBFBF\">"_S * habr_html) + u"]"_S + (u"</font>"_S * habr_html) + (ohd * u"</span></span>"_S));
            else if (ch == u'{')
                write_to_i(u"<span class=\"cu_brackets\" onclick=\"return spoiler(this, event)\"><span class=\"cu_brackets_b\">"_S * ohd + u"{"_S + (ohd * u"</span><span>…</span><span class=\"cu\" style=\"display: none\">"_S));
            else if (ch == u'}')
                write_to_i(u"</span><span class=\"cu_brackets_b\">"_S * ohd + u"}"_S + (ohd * u"</span></span>"_S));
            else if (ch == u'\n') {
                write_to_i((new_line_tag != u'\0' ? new_line_tag : u"<br />"_S) + (new_line_tag != u"" ? u"\n"_S : u""_S));
                new_line_tag = u"\0"_S;
            }
            i++;
        }
        close_ordered_list();
        close_unordered_list();
        write_to_pos(instr.len(), 0);
        assert(ending_tags.empty());
        assert(to_html_called_inside_to_html_outer_pos_list.pop() == outer_pos);

        if (outfilef == nullptr) {
            auto r = result.join(u""_S);
            if (habr_html)
                r = r.replace(u"</blockquote>\n"_S, u"</blockquote>"_S);
            return r;
        }
        return u""_S;
    }
};

template <typename T1, typename T3 = decltype(false), typename T4 = decltype(false)> auto to_html(const T1 &instr, File* const outfilef = nullptr, const T3 &ohd = false, const T4 &habr_html = false)
{
    return Converter(habr_html, ohd).to_html(instr, outfilef);
}
