"""
После данной обработки отступы перестают играть роль — границу `scope` всегда определяют фигурные скобки.
Также здесь выполняется склеивание строк, и таким образом границу statement\утверждения задаёт либо символ `;`,
либо символ новой строки (при условии, что перед ним не стоит символ `…`!).

===============================================================================================================
Ошибки:
---------------------------------------------------------------------------------------------------------------
Error: `if/else/fn/loop/switch/type` scope is empty.
---------------------------------------------------------------------------------------------------------------
Существуют операторы, которые всегда требуют нового scope\блока, который можно обозначить двумя способами:
1. Начать следующую строку с отступом относительно предыдущей, например:
if condition\условие
    scope\блок
2. Заключить блок\scope в фигурные скобки:
if condition\условие {scope\блок}

Примечание. При использовании второго способа блок\scope может иметь произвольный уровень отступа:
if condition\условие
{
scope\блок
}

---------------------------------------------------------------------------------------------------------------
Error: `if/else/fn/loop/switch/type` scope is empty, after applied implied line joining: ```...```
---------------------------------------------------------------------------------------------------------------
Сообщение об ошибке аналогично предыдущему, но выделено в отдельное сообщение об ошибке, так как может
возникать по вине ошибочного срабатывания автоматического склеивания строк (и показывается оно тогда, когда
было произведено склеивание строк в месте данной ошибки).

---------------------------------------------------------------------------------------------------------------
Error: mixing tabs and spaces in indentation: ```...```
---------------------------------------------------------------------------------------------------------------
В одной строке для отступа используется смесь пробелов и символов табуляции.
Выберите что-либо одно (желательно сразу для всего файла): либо пробелы для отступа, либо табуляцию.
Примечание: внутри строковых литералов, а также внутри строк кода можно смешивать пробелы и табуляцию. Эта
ошибка генерируется только при проверке отступа (отступ — последовательность символов разделителей от самого
начала строки до первого символа неразделителя).

---------------------------------------------------------------------------------------------------------------
Error: inconsistent indentations: ```...```
---------------------------------------------------------------------------------------------------------------
В текущей строке кода для отступа используются пробелы, а в предыдущей строке — табуляция (либо наоборот).
[[[
Сообщение было предназначено для несколько другой ошибки: для любых двух соседних строк, если взять отступ
одной из них, то другой отступ должен начинаться с него же {если отступ текущей строки отличается от отступа
предыдущей, то:
1. Когда отступ текущей строки начинается на отступ предыдущей строки, это INDENT.
2. Когда отступ предыдущей строки начинается на отступ текущей строки, это DEDENT.
}. Например:
if a:
SSTABif b:
SSTABTABi = 0
SSTABSi = 0
Последняя пара строк не удовлетворяет этому требованию, так как ни строка ‘SSTABTAB’ не начинается на строку
‘SSTABS’, ни ‘SSTABS’ не начинается на ‘SSTABTAB’.
Эта проверка имела бы смысл в случае разрешения смешения пробелов и табуляции для отступа в пределах одной
строки (а это разрешено в Python). Но я решил отказаться от этой идеи, а лучшего текста сообщения для этой
ошибки не придумал.
]]]

---------------------------------------------------------------------------------------------------------------
Error: unindent does not match any outer indentation level
---------------------------------------------------------------------------------------------------------------
[-Добавить описание ошибки.-]
===============================================================================================================
"""

new_scope_keywords = ["else", "fn", "if", "loop", "switch", "type"]
