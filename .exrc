if &cp | set nocp | endif
let s:cpo_save=&cpo
set cpo&vim
nmap gx <Plug>NetrwBrowseX
noremap j s
noremap k n
noremap l t
noremap n k
noremap s l
noremap t j
nnoremap <silent> <Plug>NetrwBrowseX :call netrw#NetrwBrowseX(expand("<cWORD>"),0)
nmap <F5> :make
let &cpo=s:cpo_save
unlet s:cpo_save
set autoindent
set autowriteall
set backspace=indent,eol
set expandtab
set helplang=en
set history=10
set matchpairs=(:),{:},[:],<:>
set nomodeline
set pastetoggle=F11
set printoptions=paper:letter
set ruler
set runtimepath=~/.vim,/var/lib/vim/addons,/usr/share/vim/vimfiles,/usr/share/vim/vim71,/usr/share/vim/vimfiles/after,/var/lib/vim/addons/after,~/.vim/after
set scrolloff=5
set shiftround
set shiftwidth=4
set smarttab
set suffixes=.bak,~,.swp,.o,.info,.aux,.log,.dvi,.bbl,.blg,.brf,.cb,.ind,.idx,.ilg,.inx,.out,.toc
set switchbuf=usetab
set updatecount=0
set wildmode=longest,list
" vim: set ft=vim :
