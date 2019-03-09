import sys
from typing import List, IO, Callable, Dict
Char = str
import syntax_highlighter_for_pqmarkup

class Exception(Exception):
    message : str
    line : int
    column : int
    pos : int
    def __init__(self, message, line, column, pos):
        self.message = message
        self.line = line
        self.column = column
        self.pos = pos

class Converter:
    to_html_called_inside_to_html_outer_pos_list : List[int]
    habr_html : bool
    ohd : bool
    highlight_style_was_added = False
    instr : str

    def __init__(self, habr_html, ohd):
        self.to_html_called_inside_to_html_outer_pos_list = []
        #self.newline_chars = []
        self.habr_html = habr_html
        self.ohd = ohd

    def instr_pos_to_line_column(self, pos):
        pos += sum(self.to_html_called_inside_to_html_outer_pos_list)
        line = 1
        line_start = -1
        t = 0
        while t < pos:
            if self.instr[t] == "\r":
                if t < pos-1 and self.instr[t+1] == "\n":
                    t += 1
                line += 1
                line_start = t
            elif self.instr[t] == "\n":
                line += 1
                line_start = t
            t += 1
        return (line, pos - line_start, pos) # returning last `pos` is necessary because `pos` was modified above [in line `pos += sum(self.to_html_called_inside_to_html_outer_pos_list)`]

    def to_html(self, instr, outfilef : IO[str] = None, *, outer_pos = 0) -> str:
        self.to_html_called_inside_to_html_outer_pos_list.append(outer_pos)

        result : List[str] = [] # this should be faster than using regular string
        class Writer:
            write : Callable[[str], None]
        outfile = Writer()
        if outfilef is None:
            outfile.write = lambda s: result.append(s)
        else:
            outfile.write = lambda s: outfilef.write(s)

        # Сохраняем instr для определения номера строки по номеру символа
        if len(self.to_html_called_inside_to_html_outer_pos_list) == 1:
            self.instr = instr

        def exit_with_error(message, pos):
            p = self.instr_pos_to_line_column(pos)
            raise Exception(message, p[0], p[1], p[2])

        i = 0
        def next_char(offset = 1):
            return instr[i + offset] if i + offset < len(instr) else Char("\0")

        def i_next_str(str): # i_ — if_/is_
            #return i + len(str) <= len(instr) and instr[i:i+len(str)] == str
            return instr[i+1:i+1+len(str)] == str # first check is not necessarily in Python

        def prev_char(offset = 1):
            return instr[i - offset] if i - offset >= 0 else Char("\0")

        def html_escape(str):
            str = str.replace('&', '&amp;').replace('<', '&lt;')
            if self.habr_html:
                str = str.replace('"', '&quot;') # нужно для корректного отображения кавычек в <a href="http://address">, так как Habr автоматически конвертирует "" в «»
            return str
        def html_escapeq(str):
            return str.replace('&', '&amp;').replace('"', '&quot;')

        writepos = 0
        def write_to_pos(pos, npos):
            nonlocal writepos
            outfile.write(html_escape(instr[writepos:pos]))
            writepos = npos

        def write_to_i(add_str, skip_chars = 1):
            write_to_pos(i, i+skip_chars)
            outfile.write(add_str)

        def find_ending_pair_quote(i): # ищет окончание ‘строки’
            assert(instr[i] == "‘") # ’
            startqpos = i
            nesting_level = 0
            while True:
                if i == len(instr):
                    exit_with_error('Unpaired left single quotation mark', startqpos)
                ch = instr[i]
                if ch == "‘":
                    nesting_level += 1
                elif ch == "’":
                    nesting_level -= 1
                    if nesting_level == 0:
                        return i
                i += 1

        def find_ending_sq_bracket(str, i, start = 0):
            starti = i
            assert(str[i] == "[") # ]
            nesting_level = 0
            while True:
                ch = str[i]
                if ch == "[":
                    nesting_level += 1
                elif ch == "]":
                    nesting_level -= 1
                    if nesting_level == 0:
                        return i
                i += 1
                if i == len(str):
                    exit_with_error('Unended comment started', start + starti)

        def remove_comments(s, start, level = 3):
            while True:
                j = s.find("["*level) # ]
                if j == -1:
                    break
                k = find_ending_sq_bracket(s, j, start) + 1
                start += k - j
                s = s[0:j] + s[k:]
            return s

        nonunique_links : Dict[int, str] = {}
        link = ''

        def write_http_link(startpos, endpos : int, q_offset = 1, text = ''):
            nonlocal i, link
            # Ищем окончание ссылки
            nesting_level = 0
            i += 2
            while True:
                if i == len(instr):
                    exit_with_error('Unended link', endpos+q_offset)
                ch = instr[i]
                if ch == "[":
                    nesting_level += 1
                elif ch == "]":
                    if nesting_level == 0:
                        break
                    nesting_level -= 1
                elif ch == " ":
                    break
                i += 1

            link = html_escapeq(instr[endpos+1+q_offset:i])
            tag : str = '<a href="' + link + '"'
            if link.startswith('./'):
                tag += ' target="_self"'

            # Ищем альтернативный текст при такой записи: ссылка[http://... ‘альтернативный текст’]
            if instr[i] == " ":
                tag += ' title="'
                if next_char() == "‘": # [
                    endqpos2 = find_ending_pair_quote(i+1)
                    if instr[endqpos2+1] != ']': # [
                        exit_with_error('Expected `]` after `’`', endqpos2+1)
                    tag += html_escapeq(remove_comments(instr[i+2:endqpos2], i+2))
                    i = endqpos2 + 1
                else:
                    endb = find_ending_sq_bracket(instr, endpos+q_offset)
                    tag += html_escapeq(remove_comments(instr[i+1:endb], i+1))
                    i = endb
                tag += '"'
            if next_char() == '[' and next_char(2) == '-':
                j = i + 3
                while j < len(instr):
                    if instr[j] == ']':
                        nonunique_links[int(instr[i+3:j])] = link
                        i = j
                        break
                    if not instr[j].isdigit():
                        break
                    j += 1
            if text == '':
                write_to_pos(startpos, i+1)
                text = html_escape(remove_comments(instr[startpos+q_offset:endpos], startpos+q_offset))
            outfile.write(tag + '>' + (text if text != '' else link) + '</a>')

        def write_note(startpos, endpos, q_offset = 1):
            nonlocal i
            i += q_offset
            endqpos2 = find_ending_pair_quote(i+1) # [[‘
            if instr[endqpos2+1] != ']':
                exit_with_error("Bracket ] should follow after ’", endqpos2+1)
            write_to_pos(startpos, endqpos2+2)
            outfile.write('<abbr title="'
                + html_escapeq(remove_comments(instr[i+2:endqpos2], i+2)) + '">'
                + html_escape(remove_comments(instr[startpos+q_offset:endpos], startpos+q_offset)) + '</abbr>')
            i = endqpos2 + 1

        endi = 0
        def numbered_link(offset = 1):
            if next_char(offset) == '-' and next_char(offset+1).isdigit():
                j = i + offset + 1
                while j < len(instr): # [
                    if instr[j] == ']':
                        nonlocal link
                        try:
                            link = nonunique_links[int(instr[i+offset+1:j])]
                        except KeyError:
                            exit_with_error("Link with such index was not declared previously", i+offset+1)
                        nonlocal endi
                        endi = j
                        return True
                    if not instr[j].isdigit():
                        break
                    j += 1
            return False

        ordered_list_current_number = -1
        def close_ordered_list():
            nonlocal ordered_list_current_number
            if ordered_list_current_number != -1:
                write_to_i("</li>\n</ol>\n", 0)
                ordered_list_current_number = -1

        in_unordered_list = False
        def close_unordered_list():
            nonlocal in_unordered_list
            if in_unordered_list:
                write_to_i("</li>\n</ul>\n", 0)
                in_unordered_list = False

        ending_tags : List[str] = []
        new_line_tag = "\0"

        while i < len(instr):
            ch = instr[i]
            if i == 0 or prev_char() == "\n": # if beginning of line
                if ch == '.' and (next_char() in ' ‘'): # ’ this is unordered list
                    close_ordered_list()
                    s = ''
                    if not in_unordered_list:
                        s = "<ul>\n<li>"
                        in_unordered_list = True
                    else:
                        s = "</li>\n<li>"
                    write_to_i(s)
                    new_line_tag = '' # используем тот факт, что разрыва строк в списках вида `. элемент списка` быть не может, и следующий символ \n будет либо закрывать список, либо обозначать начало следующего элемента списка
                    if next_char() == ' ':
                        i += 1
                    else:
                        endqpos = find_ending_pair_quote(i + 1)
                        outfile.write(self.to_html(instr[i+2:endqpos], outer_pos = i+2))
                        i = endqpos
                    writepos = i + 1
                else:
                    close_unordered_list()
                    if ch.isdigit():
                        j = i + 1
                        while j < len(instr):
                            if not instr[j].isdigit():
                                break
                            j += 1
                        if instr[j:j+1] == '.' and instr[j+1:j+2] in (' ', '‘'): # ’ this is ordered list
                            value = int(instr[i:j])
                            s = ''
                            if ordered_list_current_number == -1:
                                s = ('<ol>' if value == 1 else '<ol start="' + str(value) + '">') + "\n<li>"
                                ordered_list_current_number = value
                            else:
                                s = "</li>\n" + ("<li>" if value == ordered_list_current_number + 1 else '<li value="' + str(value) + '">')
                                ordered_list_current_number = value
                            write_to_i(s)
                            new_line_tag = '' # используем тот факт, что разрыва строк в списках вида `1. элемент списка` быть не может
                            if instr[j+1] == ' ':
                                i = j + 1
                            else:
                                endqpos = find_ending_pair_quote(j + 1)
                                outfile.write(self.to_html(instr[j+2:endqpos], outer_pos = j+2))
                                i = endqpos
                            writepos = i + 1
                        else:
                            close_ordered_list()
                    else:
                        close_ordered_list()

                if ch == ' ':
                    write_to_i('&emsp;')
                elif ch == '-': # horizontal rule
                    if i_next_str('--'):
                        j = i + 3
                        while True:
                            if j == len(instr) or instr[j] == "\n":
                                write_to_i("<hr />\n")
                                if j == len(instr):
                                    j -= 1
                                i = j
                                writepos = j + 1
                                break
                            if instr[j] != '-':
                                break
                            j += 1
                elif ch in ('>', '<') and (next_char() in ' ‘['): # this is blockquote # ]’
                    write_to_pos(i, i + 2)
                    outfile.write('<blockquote'+(ch=='<')*' class="re"'+'>')
                    if next_char() == ' ': # > Quoted text.
                        new_line_tag = '</blockquote>'
                    else:
                        if next_char() == '[': # ]
                            if numbered_link(2): # >[-1]:‘Quoted text.’
                                linkstr = link
                                if len(linkstr) > 57:
                                    linkstr = linkstr[:linkstr.rfind('/', 0, 47)+1] + '...'
                                outfile.write('<a href="' + link + '">[' + instr[i+3:endi] + ']<i>' + linkstr + '</i></a>')
                                i = endi + 1
                            else: # >[http...]:‘Quoted text.’ or >[http...][-1]:‘Quoted text.’
                                i += 1
                                endb = find_ending_sq_bracket(instr, i)
                                linkn = ''
                                if instr[endb+1:endb+3] == '[-': # ]
                                    linkn = '['+instr[endb+3:find_ending_sq_bracket(instr, endb+1)]+']'
                                link = instr[i + 1:endb]
                                spacepos = link.find(' ')
                                if spacepos != -1:
                                    link = link[:spacepos]
                                if len(link) > 57:
                                    link = link[:link.rfind('/', 0, 47)+1] + '...'
                                write_http_link(i, i, 0, linkn+'<i>'+link+'</i>') # this function changes `link` :o, but I left[‘I mean didn't rename it to `link_`’] it as is [at least for a while] because it still works correctly
                                i += 1
                            if instr[i:i+2] != ':‘': # ’
                                exit_with_error("Quotation with url should always has :‘...’ after ["+link[:link.find(':')]+"://url]", i)
                            outfile.write(":<br />\n")
                            writepos = i + 2
                        else:
                            endqpos = find_ending_pair_quote(i + 1)
                            if instr[endqpos+1:endqpos+2] == "[": # >‘Author's name’[http...]:‘Quoted text.’ # ]
                                startqpos = i + 1
                                i = endqpos
                                outfile.write('<i>')
                                assert(writepos == startqpos + 1)
                                writepos = startqpos
                                write_http_link(startqpos, endqpos)
                                outfile.write('</i>')
                                i += 1
                                if instr[i:i+2] != ':‘': # ’
                                    exit_with_error("Quotation with url should always has :‘...’ after ["+link[:link.find(':')]+"://url]", i)
                                outfile.write(":<br />\n")
                                writepos = i + 2
                            elif instr[endqpos+1:endqpos+2] == ":": # >‘Author's name’:‘Quoted text.’
                                outfile.write("<i>"+instr[i+2:endqpos]+"</i>:<br />\n")
                                i = endqpos + 1
                                if instr[i:i+2] != ':‘': # ’
                                    exit_with_error("Quotation with author's name should be in the form >‘Author's name’:‘Quoted text.’", i)
                                writepos = i + 2
                            # else this is just >‘Quoted text.’
                        ending_tags.append('</blockquote>')
                    i += 1

            if ch == "‘":
                prevci = i - 1
                prevc = instr[prevci] if prevci >= 0 else Char("\0")
                #assert(prevc == prev_char())
                startqpos = i
                i = find_ending_pair_quote(i)
                endqpos = i
                str_in_b = '' # (
                if prevc == ')':
                    openb = instr.rfind('(', 0, prevci - 1) # )
                    if openb != -1 and openb > 0:
                        str_in_b = instr[openb+1:startqpos-1]
                        prevci = openb - 1
                        prevc = instr[prevci]
                if prevc in 'PР': # Рисунок обрабатывается по-особенному
                    write_to_pos(prevci, endqpos + 1)
                    title = ''
                    endqpos2 : int
                    if i_next_str('[‘'): # альтернативный текст
                        endqpos2 = find_ending_pair_quote(i+2)
                        if instr[endqpos2+1] != ']': # [
                            exit_with_error('Expected `]` after `’`', endqpos2+1)
                        title = ' title="'+html_escapeq(remove_comments(instr[i+3:endqpos2], i+3))+'"'
                    imgtag = '<img'
                    if str_in_b != '':
                        wh = str_in_b.replace(',', ' ').split(' ')
                        assert(len(wh) in (1, 2))
                        imgtag += ' width="' + wh[0] + '" height="' + wh[-1] + '"'
                    imgtag += ' src="'+instr[startqpos+1:endqpos]+'"'+title+' />'
                    if i_next_str('[http') or i_next_str('[./'): # ]]
                        write_http_link(startqpos, endqpos, 1, imgtag)
                        writepos = i + 1
                    elif i_next_str('[‘'): # ’]
                        outfile.write(imgtag)
                        writepos = endqpos2 + 2
                        i = endqpos2 + 1
                    else:
                        outfile.write(imgtag)
                        i = endqpos
                elif i_next_str('[http') or i_next_str('[./'): # ]]
                    write_http_link(startqpos, endqpos)
                elif next_char() == '[' and numbered_link(2): # ]
                    i = endi
                    write_to_pos(startqpos, i+1)
                    outfile.write('<a href="' + link + '">' + html_escape(instr[startqpos+1:endqpos]) + '</a>')
                elif i_next_str('[‘'): # ’] сноска/альтернативный текст/текст всплывающей подсказки
                    write_note(startqpos, endqpos)
                elif next_char() == '{' and (self.habr_html or self.ohd):
                    # Ищем окончание спойлера }
                    nesting_level = 0
                    i += 2
                    while True:
                        if i == len(instr):
                            exit_with_error('Unended spoiler', endqpos+1)
                        ch = instr[i]
                        if ch == "{":
                            nesting_level += 1
                        elif ch == "}":
                            if nesting_level == 0:
                                break
                            nesting_level -= 1
                        i += 1
                    write_to_pos(prevci + 1, i + 1)
                    outer_p = endqpos+(3 if instr[endqpos+2] == "\n" else 2) # проверка на == "\n" нужна, чтобы переход на новую строку/перевод строки после `{` игнорировался
                    if self.habr_html:
                        outfile.write('<spoiler title="' + remove_comments(instr[startqpos+1:endqpos], startqpos+1).replace('"', "''") + '">\n' + self.to_html(instr[outer_p:i], outer_pos = outer_p) + "</spoiler>\n")
                    else:
                        outfile.write('<span class="spoiler_title" onclick="return spoiler2(this, event)">' + remove_comments(instr[startqpos+1:endqpos], startqpos+1) + '<br /></span>' # используется span, так как с div подчёркивание будет на весь экран
                            + '<div class="spoiler_text" style="display: none">\n' + self.to_html(instr[outer_p:i], outer_pos = outer_p) + "</div>\n")
                    if (next_char() == "\n" # чтобы переход на новую строку/перевод строки после `}` игнорировался
                            and not in_unordered_list and ordered_list_current_number == -1): # если находимся внутри списка, то пропуска новой строки делать не нужно
                        i += 1
                        writepos = i + 1
                elif prevc == "'": # raw [html] output
                    t = startqpos - 1
                    while t >= 0:
                        if instr[t] != "'":
                            break
                        t -= 1
                    eat_left = startqpos - 1 - t # количество кавычек, которые нужно съесть слева
                    t = endqpos + 1
                    while t < len(instr):
                        if instr[t] != "'":
                            break
                        t += 1
                    eat_right = t - (endqpos + 1) # количество кавычек, которые нужно съесть справа
                    write_to_pos(startqpos - eat_left, t)
                    outfile.write(instr[startqpos + eat_left:endqpos - eat_right + 1])
                elif prevc in '0OО':
                    write_to_pos(prevci, endqpos+1)
                    outfile.write(html_escape(instr[startqpos+1:endqpos]).replace("\n", "<br />\n"))
                elif prevc == "#":
                    ins = instr[startqpos+1:endqpos]
                    write_to_pos(prevci, endqpos+1)
                    if self.habr_html:
                        contains_new_line = "\n" in ins
                        outfile.write(('<source lang="' + str_in_b + '">' if str_in_b != '' else '<source>' if contains_new_line else '<code>') + ins + ("</source>" if str_in_b != '' or contains_new_line else "</code>")) # так как <source> в Habr — блочный элемент, а не встроенный\inline
                    else:
                        pre = '<pre ' + ('class="code_block"' if ins[0] == "\n" else 'style="display: inline"') + '>' # can not do `outfile.write('<pre ' + ...)` here because `outfile.write(syntax_highlighter_for_pqmarkup.css)` should be outside of <pre> block
                        if self.ohd and syntax_highlighter_for_pqmarkup.is_lang_supported(str_in_b):
                            if not self.highlight_style_was_added:
                                outfile.write(syntax_highlighter_for_pqmarkup.css)
                                self.highlight_style_was_added = True
                            try:
                                outfile.write(pre + syntax_highlighter_for_pqmarkup.highlight(str_in_b, ins) + '</pre>')
                            except syntax_highlighter_for_pqmarkup.Error as e:
                                exit_with_error('Syntax highlighter: ' + e.message, startqpos+1+e.pos)
                        else:
                            outfile.write(pre + html_escape(ins) + '</pre>') # в habr_html тег pre не стоит задействовать, так как в Habr для тега pre используется шрифт monospace, в котором символы ‘ и ’ выглядят непонятно (не так как в Courier New)
                    if ins[0] == "\n" and instr[i+1:i+2] == "\n":
                        outfile.write("\n")
                        new_line_tag = ''
                elif prevc in 'TТ':
                    write_to_pos(prevci, endqpos+1)
                    header_row = False
                    hor_row_align = ''
                    ver_row_align = ''

                    # Fill/prepare 2d-array `table`
                    class TableCell:
                        text  : str
                        attrs : str
                        def __init__(self, text : str, attrs : str): # types are needed to avoid this error in MSVC 2017: ‘error C2892: local class shall not have member templates’
                            self.text  = text
                            self.attrs = attrs
                    table : List[List[TableCell]] = []
                    j = startqpos + 1
                    while j < endqpos:
                        ch = instr[j]
                        if ch == "‘": # ’
                            empty_list : List[TableCell] = []
                            table.append(empty_list)
                            endrow = find_ending_pair_quote(j)
                            hor_col_align = ''
                            ver_col_align = ''

                            # Read table row
                            j += 1
                            while j < endrow:
                                ch = instr[j]
                                if ch == "‘": # ’
                                    end_of_column = find_ending_pair_quote(j)
                                    style = ""
                                    if hor_row_align != '' or hor_col_align != '':
                                        style += "text-align: " + (hor_col_align if hor_col_align != '' else hor_row_align)
                                    if ver_row_align != '' or ver_col_align != '':
                                        if style != "":
                                            style += "; "
                                        style += "vertical-align: " + (ver_col_align if ver_col_align != '' else ver_row_align)
                                    hor_col_align = ''
                                    ver_col_align = ''
                                    table[-1].append(TableCell(self.to_html(instr[j+1:end_of_column], outer_pos = j+1), ("th" if header_row else "td") + (' style="'+style+'"' if style != '' else '')))
                                    j = end_of_column
                                elif ch in '<>' and instr[j+1:j+2] in ('<', '>'):
                                    hor_col_align = {'<<':'left', '>>':'right', '><':'center', '<>':'justify'}[instr[j:j+2]]
                                    j += 1
                                elif instr[j:j+2] in ("/\\", "\\/"):
                                    ver_col_align = "top" if instr[j:j+2] == "/\\" else "bottom"
                                    j += 1
                                elif ch == "-":
                                    if len(table[-1]) == 0:
                                        exit_with_error('Wrong table column span marker "-"', j)
                                    table[-1].append(TableCell('', '-'))
                                elif ch == "|":
                                    if len(table) == 1:
                                        exit_with_error('Wrong table row span marker "|"', j)
                                    table[-1].append(TableCell('', '|'))
                                elif instr[j:j+3] == "[[[": # ]]]
                                    j = find_ending_sq_bracket(instr, j)
                                elif ch not in "  \t\n":
                                    exit_with_error('Unknown formatting character inside table row', j)
                                j += 1

                            header_row = False
                            hor_row_align = ''
                            ver_row_align = ''
                        elif ch in 'HН':
                            header_row = True
                        elif ch in '<>' and instr[j+1:j+2] in ('<', '>'):
                            hor_row_align = {'<<':'left', '>>':'right', '><':'center', '<>':'justify'}[instr[j:j+2]]
                            j += 1
                        elif instr[j:j+2] in ("/\\", "\\/"):
                            ver_row_align = "top" if instr[j:j+2] == "/\\" else "bottom"
                            j += 1
                        elif instr[j:j+3] == "[[[": # ]]]
                            j = find_ending_sq_bracket(instr, j)
                        elif ch not in "  \t\n":
                            exit_with_error('Unknown formatting character inside table', j)

                        j += 1

                    # Process column and row spans (walk in the reverse order — from bottom right corner of the table)
                    for y in range(len(table)-1, -1, -1):
                        for x in range(len(table[y])-1, -1, -1):
                            if table[y][x].attrs in ('-', '|'):
                                xx = x
                                yy = y
                                while True:
                                    if table[yy][xx].attrs == '-':
                                        xx -= 1
                                    elif table[yy][xx].attrs == '|':
                                        yy -= 1
                                    else:
                                        break
                                if xx < x:
                                    table[yy][xx].attrs += ' colspan="'+str(x-xx+1)+'"'
                                if yy < y:
                                    table[yy][xx].attrs += ' rowspan="'+str(y-yy+1)+'"'
                                for xxx in range(xx, x+1): # mark a whole rect of this merged cell as processed to avoid its further processing (in this loop) and to skip it at output table loop
                                    for yyy in range(yy, y+1):
                                        if (xxx, yyy) != (xx, yy):
                                            table[yyy][xxx].attrs = ''

                    # Output table
                    is_inline = True
                    if (prevci == 0 or
                        instr[prevci-1] == "\n" or # [[[
                       (prevci-3 >= 0 and instr[prevci-3:prevci] == ']]]' and instr[0:3] == '[[[' and find_ending_sq_bracket(instr, 0) == prevci-1)): # ]]]
                        is_inline = False
                    outfile.write("<table"+' style="display: inline"'*is_inline+">\n")
                    for row in table:
                        outfile.write("<tr>")
                        for cell in row:
                            if cell.attrs != '': # if this is a merged cell (cell.attrs == '') — skip it
                                outfile.write('<' + cell.attrs + '>' + cell.text + '</' + cell.attrs[:2] + '>')
                        outfile.write("</tr>\n")
                    outfile.write("</table>\n")
                    if not is_inline:
                        new_line_tag = ''
                elif prevc in '<>' and instr[prevci-1] in '<>': # выравнивание текста \ text alignment
                    write_to_pos(prevci-1, endqpos+1)
                    outfile.write('<div align="' + {'<<':'left', '>>':'right', '><':'center', '<>':'justify'}[instr[prevci-1]+prevc] + '">'
                                 + self.to_html(instr[startqpos+1:endqpos], outer_pos = startqpos+1) + "</div>\n")
                    new_line_tag = ''
                elif i_next_str(":‘") and instr[find_ending_pair_quote(i+2)+1:][:1] == '<': # this is reversed quote ‘Quoted text.’:‘Author's name’< # ’
                    endrq = find_ending_pair_quote(i+2)
                    i = endrq + 1
                    write_to_pos(prevci + 1, i + 1)
                    outfile.write('<blockquote>' + self.to_html(instr[startqpos+1:endqpos], outer_pos = startqpos+1) + "<br />\n<div align='right'><i>" + instr[endqpos+3:endrq] + "</i></div></blockquote>")
                    new_line_tag = ''
                else:
                    i = startqpos # откатываем позицию обратно
                    if prev_char() in '*_-~':
                        write_to_pos(i - 1, i + 1)
                        tag = {'*':'b', '_':'u', '-':'s', '~':'i'}[prev_char()]
                        outfile.write('<' + tag + '>')
                        ending_tags.append('</' + tag + '>')
                    elif prevc in 'HН':
                        write_to_pos(prevci, i + 1)
                        tag = 'h' + str(min(max(3 - (0 if str_in_b == '' else int(str_in_b)), 1), 6))
                        outfile.write('<' + tag + '>')
                        ending_tags.append('</' + tag + '>')
                    elif prevc in 'CС':
                        write_to_pos(prevci, i + 1)
                        which_color = 'color'
                        if str_in_b[0:1] == '-':
                            str_in_b = str_in_b[1:]
                            which_color = 'background-color'
                        if str_in_b[0:1] == '#':
                            new_str_in_b = ''
                            for c in str_in_b:
                                cc = {'а':'A','б':'B','с':'C','д':'D','е':'E','ф':'F'}.get(c.lower(), c)[0]
                                new_str_in_b += cc.lower() if c.islower() else cc
                            str_in_b = new_str_in_b
                        elif len(str_in_b) in (1, 3) and str_in_b.isdigit():
                            new_str = "#"
                            for ii in [0, 1, 2] if len(str_in_b) == 3 else [0, 0, 0]:
                                new_str += hex((int(str_in_b[ii]) * 0xFF + 4) // 8)[2:].upper().zfill(2) # 8 - FF, 0 - 00, 4 - 80 (почему не 7F[‘когда `+ 3` вместо `+ 4`’] — две субъективные причины: 1.‘больше нравится как выглядит’ и 2.‘количество пикселей в строке `80` при `"font_face": "Courier New", "font_size": 10`’)
                            str_in_b = new_str
                        if self.habr_html:
                            outfile.write('<font color="' + str_in_b + '">')
                            ending_tags.append('</font>')
                        else: # The <font> tag is not supported in HTML5.
                            outfile.write('<span style="'+which_color+': ' + str_in_b + '">')
                            ending_tags.append('</span>')
                    elif (instr[prevci-1:prevci], prevc) in (('/', "\\"), ("\\", '/')):
                        write_to_pos(prevci-1, i + 1)
                        tag = 'sup' if (instr[prevci-1], prevc) == ('/', "\\") else 'sub'
                        outfile.write('<' + tag + '>')
                        ending_tags.append('</' + tag + '>')
                    elif prevc == '!':
                        write_to_pos(prevci, i + 1)
                        outfile.write('<blockquote>' if self.habr_html else '<div class="note">')
                        ending_tags.append('</blockquote>' if self.habr_html else '</div>')
                    else: # ‘
                        ending_tags.append('’')
            elif ch == "’":
                write_to_pos(i, i + 1)
                if len(ending_tags) == 0:
                    exit_with_error('Unpaired right single quotation mark', i)
                last = ending_tags.pop()
                outfile.write(last)
                if next_char() == "\n" and (last.startswith('</h') or last in ('</blockquote>', '</div>')): # так как <h.> - блоковый элемент, то он автоматически завершает строку, поэтому лишний тег <br> в этом случае добавлять не нужно (иначе получится лишняя пустая строка после заголовка)
                    outfile.write("\n")
                    i += 1
                    writepos += 1
            elif ch == '`':
                # Сначала считаем количество символов ` — это определит границу, где находится окончание span of code
                start = i
                i += 1
                while i < len(instr):
                    if instr[i] != '`':
                        break
                    i += 1
                end = instr.find((i - start)*'`', i)
                if end == -1:
                    exit_with_error('Unended ` started', start)
                write_to_pos(start, end + i - start)
                ins = instr[i:end]
                delta = ins.count("‘") - ins.count("’") # в `backticks` могут быть ‘кавычки’ и в [[[комментариях]]] (выглядит это, например, так: [[[‘]]]`Don’t`), для этого и нужны
                if delta > 0: # эти строки кода[:backticks]
                    for ii in range(delta): # ‘‘
                        ending_tags.append('’')
                else:
                    for ii in range(-delta):
                        if ending_tags.pop() != '’':
                            exit_with_error('Unpaired single quotation mark found inside code block/span beginning', start)
                ins = html_escape(ins)
                if not "\n" in ins: # this is a single-line code -‘block’span
                    outfile.write('<code>' + ins + '</code>' if self.habr_html else '<pre class="inline_code">' + ins + '</pre>')
                else:
                    outfile.write('<pre>' + ins + '</pre>' + "\n"*(not self.habr_html))
                    new_line_tag = ''
                i = end + i - start - 1
            elif ch == '[': # ]
                if i_next_str('http') or i_next_str('./') or (i_next_str('‘') and prev_char() not in "\r\n\t \0") or numbered_link(): # ’
                    s = i - 1
                    while s >= writepos and instr[s] not in "\r\n\t [{(‘“": # ”’)}]
                        s -= 1
                    if i_next_str('‘'): # ’ сноска/альтернативный текст/текст всплывающей подсказки
                        write_note(s + 1, i, 0)
                    elif i_next_str('http') or i_next_str('./'):
                        write_http_link(s + 1, i, 0)
                    else:
                        write_to_pos(s + 1, endi+1)
                        outfile.write('<a href="' + link + '">' + html_escape(instr[s+1:i]) + '</a>')
                        i = endi
                elif i_next_str('[['): # ]] comment
                    comment_start = i
                    nesting_level = 0
                    while True:
                        ch = instr[i]
                        if ch == "[":
                            nesting_level += 1
                        elif ch == "]":
                            nesting_level -= 1
                            if nesting_level == 0:
                                break
                        elif ch == "‘": # [backticks:]а также эти строки кода
                            ending_tags.append('’') # ‘‘
                        elif ch == "’":
                            assert(ending_tags.pop() == '’')
                        i += 1
                        if i == len(instr):
                            exit_with_error('Unended comment started', comment_start)
                    write_to_pos(comment_start, i+1)
                    if instr[comment_start+3:comment_start+4] != '[': # это [[[такой]]] комментарий, а не [[[[такой]]]] или [[[[[такой и [[[[[[так далее]]]]]]]]]]], а [[[такие]]] комментарии следует транслировать в HTML: <!--[[[комментарий]...]...]-->
                        outfile.write('<!--')
                        outfile.write(remove_comments(instr[comment_start:i+1], comment_start, 4)) # берётся вся строка вместе со [[[скобочками]]] для [[[таких] ситуаций]]
                        outfile.write('-->')
                else:
                    write_to_i('<span class="sq"><span class="sq_brackets">'*self.ohd + '<font color="#BFBFBF">'*self.habr_html + '[' + '</font><font color="gray">'*self.habr_html + self.ohd*'</span>')
            elif ch == "]":
                write_to_i('<span class="sq_brackets">'*self.ohd + '</font><font color="#BFBFBF">'*self.habr_html + ']' + '</font>'*self.habr_html + self.ohd*'</span></span>')
            elif ch == "{":
                write_to_i('<span class="cu_brackets" onclick="return spoiler(this, event)"><span class="cu_brackets_b">'*self.ohd + '{' + self.ohd*'</span><span>…</span><span class="cu" style="display: none">')
            elif ch == "}":
                write_to_i('</span><span class="cu_brackets_b">'*self.ohd + '}' + self.ohd*'</span></span>')
            elif ch == "\n":
                write_to_i((new_line_tag if new_line_tag != "\0" else "<br />") + ("\n" if new_line_tag != '' else "")) # код `"\n" if new_line_tag != ''` нужен только для списков (unordered/ordered list)
                new_line_tag = "\0"

            i += 1

        close_ordered_list()
        close_unordered_list()

        write_to_pos(len(instr), 0)
        assert(len(ending_tags) == 0) # ‘слишком много открывающих одинарных кавычек’/‘где-то есть незакрытая открывающая кавычка’

        assert(self.to_html_called_inside_to_html_outer_pos_list.pop() == outer_pos)

        if outfilef is None:
            r = "".join(result)
            if self.habr_html:                                    # // dirty hack
                r = r.replace("</blockquote>\n", '</blockquote>') # \\ (just left it as is)
            return r

        return ''

def to_html(instr, outfilef : IO[str] = None, ohd = False, *, habr_html = False):
    return Converter(habr_html, ohd).to_html(instr, outfilef)


if __name__ == '__main__':
    # Support running module as a command line command.
    if '-h' in sys.argv or '--help' in sys.argv:
        print(R'''A Python implementation of pq markup to HTML converter.

Usage: pqmarkup [options] [INPUTFILE]

Positional arguments:
  INPUT_FILE            input file (STDIN is assumed if no INPUT_FILE is
                        given)

Options:
  -h, --help            show this help message and exit
  --habr-html           for publishing posts on habr.com
  --output-html-document
                        add some html header for rough testing preview of your
                        converted documents
  -f [OUTPUT_FILE], --file [OUTPUT_FILE]
                        write output to OUTPUT_FILE (defaults to STDOUT)''')
        sys.exit(0)

    args_habr_html            = '--habr-html'            in sys.argv
    args_output_html_document = '--output-html-document' in sys.argv
    args_infile = sys.stdin
    i = 1
    while i < len(sys.argv):
        if sys.argv[i] in ('-f', '--file'):
            i += 2
            continue
        if not sys.argv[i].startswith('-'):
            args_infile = open(sys.argv[i], 'r', encoding = 'utf-8-sig')
            break
        i += 1
    args_outfile = sys.stdout
    if '-f' in sys.argv:
        args_outfile = open(sys.argv[sys.argv.index('-f')     + 1], 'w', encoding = 'utf-8', newline = "\n")
    elif '--file' in sys.argv:
        args_outfile = open(sys.argv[sys.argv.index('--file') + 1], 'w', encoding = 'utf-8', newline = "\n")

    if args_output_html_document and args_habr_html:
        sys.exit("Options --output-html-document and --habr-html are mutually exclusive")

    infile_str = args_infile.read()
    title = ''
    if infile_str.startswith('[[[H‘') or \
       infile_str.startswith('[[[Н‘'): # ’]]]’]]]
        i = 5
        nesting_level = 1
        while i < len(infile_str):
            ch = infile_str[i]
            if ch == "‘":
                nesting_level += 1
            elif ch == "’":
                nesting_level -= 1
                if nesting_level == 0:
                    break
            i += 1
        title = infile_str[5:i]

    if args_output_html_document:
        args_outfile.write(
R'''<html>
<head>
<meta charset="utf-8" />
''' + ('<title>' + title + "</title>\n" if title != '' else '') + R'''<base target="_blank">
<script type="text/javascript">
function spoiler(element, event)
{
    if (event.target.nodeName == 'A' || event.target.parentNode.nodeName == 'A' || event.target.onclick)//чтобы работали ссылки в спойлерах и спойлеры2 в спойлерах
        return;
    var e = element.firstChild.nextSibling.nextSibling;//element.getElementsByTagName('span')[0]
    e.previousSibling.style.display = e.style.display;//<span>…</span> must have inverted display style
    e.style.display = (e.style.display == "none" ? "" : "none");
    element.firstChild.style.fontWeight =
    element. lastChild.style.fontWeight = (e.style.display == "" ? "normal" : "bold");
    event.stopPropagation();
    //[-Чтобы была возможность выделять текст внутри раскрытого ‘скрытого текста’/спойлера, необходимо [скрытие/]закрытие спойлера делать не просто по нажатию левой кнопки мыши, а по отпусканию левой кнопки мыши при условии отсутствия движения курсора [мыши] после того, как была нажата левая кнопка мыши.-]
}
function spoiler2(element, event)
{
    element.nextSibling.style.display = (element.nextSibling.style.display == "none" ? "" : "none");
}
</script>
<style type="text/css">
body td {
    font-size: 14px;
    font-family: Verdana, sans-serif;
    line-height: 160%;
}
span.cu_brackets_b {
    font-size: initial;
    font-family: initial;
    font-weight: bold;
}
a {
    text-decoration: none;
    color: #6da3bd;
}
a:hover {
    text-decoration: underline;
    color: #4d7285;
}
h1, h2, h3, h4, h5, h6 {
    margin: 0;
    font-weight: 400;
}
h1 {font-size: 190%; line-height: 120%;}
h2 {font-size: 160%;}
h3 {font-size: 137.5%;}
h4 {font-size: 120%;}
h5 {font-size: 110%;}
h6 {font-size: 100%;}
td {text-align: justify; /*font-family: Tahoma, sans-serif;*/}
span.sq {color: gray; font-size: 0.8rem; font-weight: normal; /*pointer-events: none;*/}
span.sq_brackets {color: #BFBFBF;}
span.cu_brackets {cursor: pointer;}
span.cu {background-color: #F7F7FF;}
abbr {text-decoration: none; border-bottom: 1px dotted;}
pre {margin: 0;}''' + # когда везде использовался <pre style="display: inline">, то margin в таких блоках не учитывался, поэтому и без этой строки с `pre {margin: 0}` код выглядел также как с этой строкой выглядят `<pre>` без `style="display: inline"`; но, вообще говоря, я добавил эту строку для соответствия форматированию Habr
'''
pre, code {font-family: 'Courier New'; line-height: normal}
ul, ol {margin: 11px 0 7px 0;}
ul li, ol li {padding: 7px 0;}
ul li:first-child, ol li:first-child {padding-top   : 0;}
ul  li:last-child, ol  li:last-child {padding-bottom: 0;}
table table {margin: 9px 0; border-collapse: collapse;}
table table th, table table td {padding: 6px 13px; border: 1px solid #BFBFBF;}
span.spoiler_title {
    color: #548eaa;
    cursor: pointer;
    border-bottom: 1px dotted;
}
div.spoiler_text {
    /*border: 1px dotted;*/
    margin: 5px;
    padding: 3px;
}
blockquote {
    margin: 0 0 7px 0;
    padding: 7px 12px;
}
blockquote:not(.re) {border-left:  0.2em solid #90ddaa; background-color: #fbfffb;}
blockquote.re       {border-right: 0.2em solid #90ddaa; background-color: #f4fff8;}
div.note {
    padding: 18px 20px;
    background: #ffffd7;
}
pre.code_block {padding: 6px 0;}
pre.inline_code {
    display: inline;
    padding: 0px 3px;
    border: 1px solid #E5E5E5;
    background-color: #FAFAFA;
    border-radius: 3px;
}
img {vertical-align: middle;}
</style>
</head>
<body>
<table width="55%" align="center"><tr><td>
''')
    try:
        to_html(infile_str, args_outfile, args_output_html_document, habr_html = args_habr_html)
    except Exception as e:
        sys.stderr.write(e.message + " at line " + str(e.line) + ", column " + str(e.column) + "\n")
        sys.exit(-1)
    if args_output_html_document:
        args_outfile.write('''
</td></tr></table>
</body>
</html>''')
