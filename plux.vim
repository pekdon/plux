" Vim syntax file
" Language: plux
" Maintainer: Claes Nästén

" quit when a syntax file was already loaded
if exists("b:current_syntax")
	finish
endif

" Keywords
syntax keyword pluxKeyword cleanup doc enddoc function endfunction call shell
syntax keyword pluxDebug log
syntax keyword pluxInclude include
syntax keyword pluxDefine local global

" Matches
syntax match pluxComment "#.*$"
syntax match pluxInput "^\s*!.*$"
syntax match pluxMatch "^\s*?{1,3}.*$"

" Regions
syntax region pluxDoc start="\[doc\]" end="\[enddoc\]"
" syntax region pluxFunction start="\[function " end="\[endfunction\]"

" Hightlight
highlight link pluxComment Comment
highlight link pluxDoc Comment
highlight link pluxDebug Debug
highlight link pluxDefine Define
highlight link pluxInclude Include
highlight link pluxInput Statement
highlight link pluxKeyword Keyword

let b:current_syntax = "plux"
