; 
; irit.el - Definitions of IRIT mode for emacs editor.
; 
; Author:	Gershon Elber
; 		Computer Science Dept.
; 		Technion, Israel Institute of technology
; Date:	Tue May 14 1991
; Copyright (c) 1991, Gershon Elber
;
; This file defines an environment to run edit and execute IRIT programs.
; Such a program should have a '.irt' extension in order it to be in
; irit-mode major mode. Several new functions are provided to communicate
; between the editted file and the solid modeller:
;
; 1. send-line-to-irit - sends a single line to the solid modeller for
;    execution. A line is defined from current position to the next
;    semicolon ';'. If however several commands exists on the same line
;    they will all be send as one line.
;    Bounded to Meta-E by default.
; 2. send-region-to-irit - sends the region from the current mark (mark-marker)
;    to current position (point-marker) to the solid modeller. This function
;    is convenient for sending a large block of commands.
;    Bounded to Meta-R by default.
; 3. send-mini-buffer-to-irit - sends a line retrieved via the mini buffer to
;    the solid modeller for execution. The line is appended with a new line
;    and is echoed to the irit-solid-modeller buffer if irit-echo-program.
;    Bounded to Meta-S by default.
;
; Both functions checks for existance of a buffer named irit-solid-modeller
; and a process named "irit" hooked to it, and will restart a new process
; or buffer if none exists. The program to execute as process "irit" is
; defined by the irit-program constant below.
;
; Mod: Martin Glanvill 15/8/95 (emacs 19.29 & xemacs 19.12)
; Place this file in emacs site-lisp dir and Add: 
;       (require 'irit)
;       (add-hook 'irit-mode-hook 'turn-on-font-lock t)
;       (add-hook 'irit-mode-hook 'outline-minor-mode t)
;
; to your ~/.emacs to get syntax highlighting

;(require 'c-mode)
;(require 'cc-mode)

(defvar irit-program "irit.exe"
  "*The executable to run for irit-solid-modeller buffer.")

(defvar irit-echo-program nil
  "*Control echo of executed commands to irit-solid-modeller buffer.")

(defvar irit-mode-map nil "")
(if irit-mode-map
    ()
  (setq irit-mode-map (make-sparse-keymap))
  (define-key irit-mode-map "\M-s" 'send-mini-buffer-to-irit)
  (define-key irit-mode-map "\M-e" 'send-line-to-irit)
  (define-key irit-mode-map "\M-r" 'send-region-to-irit)
  (define-key irit-mode-map "\M-h" 'send-word-to-irit-help))

(define-key c-mode-map "\M-o" 'make-irit-c-function)
;(define-key c++-mode-map "\M-o" 'make-irit-c-function)
(define-key c-mode-map "\M-p" 'overwrite-mode)
;(define-key c++-mode-map "\M-p" 'overwrite-mode)

;;;
;;; The following defines a small subset of keywords that 
;;; are higlighted when fontlock is set.
;;; To add extra keywords: [between the '\\<\\(' and '\\)\\>']
;;; just add:   |<keyword>\\
;;; See Emacs documentation for more info.
;;;
(defconst irit-font-lock-keywords  
'("\\<\\(box\\|cylin\\|arc\\|rotx\\|roty\\|rotz\\|poly\\|cone\\|\
   true\\|false\\|save\\|interact\\|view\\|circle\\|ctlpt\\|\
   convex\\|torus\\|vector\\|list\\|extrude\\|for\\|free\\)\\>")
" define regexp syntax keywords for font highlighing ")

(defvar irit-mode-syntax-table nil "")
(defvar irit-mode-symbol-syntax-table nil "")

;;;
;;; Add fontlock hook
;;;
(add-hook 'irit-mode-hook
 '(lambda () (make-local-variable 'font-lock-defaults)
               (setq font-lock-defaults '(irit-font-lock-keywords nil nil))))

;;;
;;; Boolean function to test if running xemacs or lucid emacs
;;;
(defun irit-xemacs ()
  (or (string-match "Lucid"  emacs-version)
      (string-match "XEmacs" emacs-version)))

;;;
;;; Creates asthetic menu (probably redundant) when changing to 
;;; irit mode.
;;; 
;;; All major modes have a menu - so why not irit?!?
;;; works with xemacs 19.12 & emacs 19.29
;;;
;(defun irit-add-irit-menu ()
;  "Adds the menu 'irit' to the menu-bar in Irit Mode."
; 
; (cond
;  ((irit-xemacs)
;    (add-menu nil "Irit"
;            '(["Send line to Irit... " send-line-to-irit t]
;              ["Send region to Irit..." send-region-to-irit  t]
;	      ["Send buffer to Irit..." send-mini-buffer-to-irit t])))
;  (t
;   (require 'easymenu)
;   (easy-menu-define irit-mode-menu irit-mode-map "Menu keymap for Irit mode."
;                    '("Irit"
;                      ["Send line to Irit... " send-line-to-irit t]
;                      ["Send region to Irit..." send-region-to-irit  t]
;		      ["Send buffer to Irit..." send-mini-buffer-to-irit t])))))


;;;
;;; Modify the syntax table for irit comments
;;; and define what is punctuation & what's not
;;; (comment fontlock seems to work in xemacs,
;;; but may need further testing - whipped out
;;; of ada-mode.el)
;;;
(defun irit-create-syntax-table ()
  "Create the syntax table for irit-mode."
  ;; There are two different syntax-tables.  The standard one declares
  ;; `_' a symbol constituent, in the second one, it is a word
  ;; constituent.  For some search and replacing routines we
  ;; temporarily switch between the two.
  (setq irit-mode-syntax-table (make-syntax-table))
  (set-syntax-table  irit-mode-syntax-table)
  (modify-syntax-entry ?\" "\"" irit-mode-syntax-table)
  (modify-syntax-entry ?\# "<" irit-mode-syntax-table)
  (modify-syntax-entry ?\n ">" irit-mode-syntax-table)
  (modify-syntax-entry ?:  "." irit-mode-syntax-table)
  (modify-syntax-entry ?\; "." irit-mode-syntax-table)
  (modify-syntax-entry ?&  "." irit-mode-syntax-table)
  (modify-syntax-entry ?\|  "." irit-mode-syntax-table)
  (modify-syntax-entry ?+  "." irit-mode-syntax-table)
  (modify-syntax-entry ?*  "." irit-mode-syntax-table)
  (modify-syntax-entry ?/  "." irit-mode-syntax-table)
  (modify-syntax-entry ?=  "." irit-mode-syntax-table)
  (modify-syntax-entry ?<  "." irit-mode-syntax-table)
  (modify-syntax-entry ?>  "." irit-mode-syntax-table)
  (modify-syntax-entry ?$ "." irit-mode-syntax-table)
  (modify-syntax-entry ?\[ "." irit-mode-syntax-table)
  (modify-syntax-entry ?\] "." irit-mode-syntax-table)
  (modify-syntax-entry ?\{ "." irit-mode-syntax-table)
  (modify-syntax-entry ?\} "." irit-mode-syntax-table)
  (modify-syntax-entry ?. "." irit-mode-syntax-table)
  (modify-syntax-entry ?\\ "." irit-mode-syntax-table)
  (modify-syntax-entry ?\' "." irit-mode-syntax-table)
  (modify-syntax-entry ?-  "." irit-mode-syntax-table)

  ;; define what belongs in irit symbols
  (modify-syntax-entry ?_ "_" irit-mode-syntax-table)

  ;; define parentheses to match
  (modify-syntax-entry ?\( "()" irit-mode-syntax-table)
  (modify-syntax-entry ?\) ")(" irit-mode-syntax-table)

  (setq irit-mode-symbol-syntax-table (copy-syntax-table irit-mode-syntax-table))
  (modify-syntax-entry ?_ "w" irit-mode-symbol-syntax-table)
  )

;;;
;;; Define the irit-mode
;;;
;;; Instantiate the menu & syntax table (with parenthesis matching).
;;;
(defun irit-mode ()
  "Major mode for editing and executing IRIT files.

see send-line-to-irit and send-region-to-irit for more."
  (interactive)
  (kill-all-local-variables)
  (use-local-map irit-mode-map)
  (setq major-mode 'irit-mode)
  (setq mode-name "Irit")
; (irit-add-irit-menu)
  (make-local-variable 'fill-column)
  (setq fill-column 75)
  (setq blink-matching-paren t)
  (if irit-mode-syntax-table
      (set-syntax-table irit-mode-syntax-table)
    (irit-create-syntax-table))
  (run-hooks 'irit-mode-hook))

;;;
;;; Internal function send the string to IRIT in small chunks - Otherwise
;;; Emacs-IRIT IPC hangs for some unknown reason.
;;;
(defun send-string-to-irit (str)
  (if (< (length str) 100)
    (let* ((crnt-buffer (buffer-name)))
      (process-send-string "irit" str)
      (switch-to-buffer-other-window (get-buffer "irit-solid-modeller"))
      (recenter -2)
      (switch-to-buffer-other-window (get-buffer crnt-buffer)))
    (progn
      (process-send-string "irit" (substring str 0 100))
      (send-string-to-irit (substring str 100)))))

;;;
;;; Define send-min-buffer-to-irit - send one line prompt for at the mini
;;; buffer, to the irit buffer.
;;;
(defun send-mini-buffer-to-irit ()
  "Sends one line of code from mini-buffer to the IRIT program.

The IRIT solid modeller buffer name is irit-solid-modeller and the 
process name is 'irit'. If none exists, a new one is created.

The name of the irit program program to execute is stored in irit-program
and may be changed."
  (interactive)
  (if (equal major-mode 'irit-mode)
    (progn
      (make-irit-buffer)     ; In case we should start a new one.
      (let* ((crnt-buffer (buffer-name))
	     (string-copy (read-from-minibuffer "Irit> ")))
	(switch-to-buffer-other-window (get-buffer "irit-solid-modeller"))
	(end-of-buffer)
	(if irit-echo-program
	  (insert string-copy))
	;(process-send-string "irit" string-copy)
	(send-string-to-irit string-copy)
	(process-send-string "irit" "\n")
	(switch-to-buffer-other-window (get-buffer crnt-buffer))))
    (message "Should be invoked in irit-mode only.")))
;;;
;;; Define send-line-to-irit - send from current cursor position to next
;;; semicolon detected.
;;;
(defun send-line-to-irit ()
  "Sends one line of code from current buffer to the IRIT program.

Use to execute a line in the IRIT solid modeller. A line is anything
that is terminated by a semicolon, but is at least one line of text so
multiple commands per line (with several semicolons) are still
considered a single line.

The IRIT solid modeller buffer name is irit-solid-modeller and the 
process name is 'irit'. If none exists, a new one is created.

The name of the irit program program to execute is stored in irit-program
and may be changed."
  (interactive)
  (if (equal major-mode 'irit-mode)
    (progn
      (make-irit-buffer)        ; In case we should start a new one.
      (beginning-of-line)
      (let ((start-mark (point-marker)))
	(search-forward ";")
	(let ((end-one-mark (point-marker)))
	  (goto-char start-mark)
	  (beginning-of-line)
	  (next-line 1)
	  (let* ((crnt-buffer (buffer-name))
	         (end-two-mark (point-marker))
	         (end-max-mark (max end-one-mark end-two-mark))
		 (string-copy (buffer-substring start-mark end-max-mark)))
	    (switch-to-buffer-other-window (get-buffer "irit-solid-modeller"))
	    (end-of-buffer)
	    (if irit-echo-program
	      (insert string-copy))
	    (set-marker (process-mark (get-process "irit")) (point-marker))
	    (switch-to-buffer-other-window (get-buffer crnt-buffer))
	    (send-string-to-irit string-copy)
	    ;(process-send-string "irit" string-copy)
	    ;(process-send-region "irit" start-mark end-max-mark)
	    (goto-char end-max-mark)
	    (if (equal "\n" (buffer-substring (point-marker)
					      (+ 1 (point-marker))))
	      (process-send-string "irit" "\n"))  
	    (if (> end-one-mark end-two-mark)
	      (forward-char 1))))))
    (message "Should be invoked in irit-mode only.")))

;;;
;;; Define send-region-to-irit - send from current cursor position to
;;; current marker.
;;;
(defun send-region-to-irit ()
  "Sends a region of code from current buffer to the IRIT program.

When this function is invoked on an IRIT file it send the region from current
point to current mark to the irit solid modeller.

The IRIT solid modeller buffer name is irit-solid-modeller and the
process name is 'irit'. If none exists, a new one is created.

The name of the irit program program to execute is stored in irit-program
and may be changed."
  (interactive)
  (if (equal major-mode 'irit-mode)
    (progn
      (make-irit-buffer)     ; In case we should start a new one.
      (copy-region-as-kill (mark-marker) (point-marker))
      (let ((crnt-buffer (buffer-name)))
	(switch-to-buffer-other-window (get-buffer "irit-solid-modeller"))
	(end-of-buffer)
	(if irit-echo-program
	  (yank))
	(set-marker (process-mark (get-process "irit")) (point-marker))
	(switch-to-buffer-other-window (get-buffer crnt-buffer))
	;(process-send-region "irit" (mark-marker) (point-marker))))
	(send-string-to-irit (buffer-substring (mark-marker) (point-marker)))))
    (message "Should be invoked in irit-mode only.")))

;;;
;;; Switch to "irit-solid-modeller" buffer if exists. If not, creates one and
;;; execute the program defined by irit-program.
;;;
(defun make-irit-buffer ()
  "Switch to irit-solid-modeller buffer or create one if none exists"
  (interactive)
  (if (not (get-process "irit"))
    (start-process "irit" "irit-solid-modeller" irit-program)))

;;;
;;; Get help on a word from irit.
;;;
(defun send-word-to-irit-help ()
   "Sends the current word to the help mechanism of irit"
  (interactive)
  (if (equal major-mode 'irit-mode)
    (progn
      (make-irit-buffer)     ; In case we should start a new one.
      (send-string-to-irit (concat "help(\"" (current-word) "\");\n")))
    (message "Should be invoked in irit-mode only.")))

;;;
;;; Autoload irit-mode on any file with irt extension. 
;;;
(setq auto-mode-alist (append '(("\\.irt$" . irit-mode))
			      auto-mode-alist))

;;;
;;; Gets a single function's parameter containing both type andname and
;;; isolate the parametr's name out of it.
;;;
(defun make-irit-c-isolate-var-name (type-and-name)
  (let* ((match1 (string-match "[^\*^& 	]+" type-and-name))
	 (match2 (string-match "[\[ 	]+" type-and-name match1))
	 (match3 (string-match "[^\*^& 	]+" type-and-name match2))
	 (match4 (string-match "[\[ 	]+" type-and-name match3)))
    (if match3
      (substring type-and-name match3 (if match4
					match4
					(length type-and-name)))
      (substring type-and-name match1 (if match2
					match2
					(length type-and-name))))))

;;;
;;; Given a whole describing the arguments, split it into the individual
;;; parameters by searching for commas and close parenthesis.
;;;
(defun make-irit-c-func-parse-args (args)
  (if (or (string-match "([ 	]*void[ 	]*)" args)
	  (string-match "([ 	]*)" args))
    nil
    (let ((match (string-match "[,)]" args)))
      (if match
	(cons (make-irit-c-isolate-var-name (substring args 1 match))
	      (make-irit-c-func-parse-args
	                     (substring args (+ match 1) (length args))))))))

;;;
;;; Insert a list describing the arguments into the current buffer.
;;;
(defun make-irit-c-func-gen-param-max-len(args)
  (if args
    (max (length (car args)) (make-irit-c-func-gen-param-max-len (cdr args)))
    0))

(defun make-irit-c-func-ins-params (args term-ch)
  (let* ((param-max-len (+ 1 (make-irit-c-func-gen-param-max-len args))))
    (if args
      (progn
        (insert (concat "*   " (car args) ":"))
        (insert-char ?  (- param-max-len (length (car args))))
        (insert "N.S.F.I.")
        (insert-char ?  (- 64 param-max-len))
        (insert term-ch)
        (make-irit-c-func-ins-params (cdr args) term-ch)))))

;;;
;;; Filters out trailing spaces if it is not a pointer (no * or &)
;;;
(defun make-irit-c-func-retval (retval)
  (let* ((match1 (string-match "[*&]+" retval))
	 (match2 (string-match "[ 	]+" retval)))
    (if match1
      retval
      (if match2
        (substring retval 0 match2)
	retval))))

;;;
;;; Make a skeleton header for an IRIT C source function.
;;;
(defun make-irit-c-function ()
  "Creates a sketelon for a C function for the IRIT solid modeler C code"
  (interactive)
  (if (or (equal major-mode 'c-mode) (equal major-mode 'c++-mode))
    (progn
      (let* ((func-proto (read-from-minibuffer "Function Prototype: "))
	     (match1 (string-match "[ 	]+" func-proto))
	     (match2 (string-match "[^* 	]+" func-proto match1))
	     (match3 (string-match "[ 	]+" func-proto match2))
	     (match4 (string-match "[^* 	]+" func-proto match3))
	     (match5 (string-match "(" func-proto))
	     (ret-val (if (and match3 (< match3 match5))
			(make-irit-c-func-retval (substring func-proto match2
							              match4))
			(make-irit-c-func-retval (substring func-proto 0
                                                                      match2))))
	     (ret-val-insert (if (equal "" ret-val)
				"NoValue"
			        ret-val))
	     (term-ch (if (string-match "static" (substring func-proto 0 match1))
			"*\n"
			"M\n"))
	     (func-name (if (and match3 (< match3 match5))
			  (substring func-proto match4 match5)
			  (substring func-proto match2 match5)))
	     (args (make-irit-c-func-parse-args
		    (substring func-proto match5 (length func-proto)))))
        (insert "/*****************************************************************************\n")
        (insert (concat "* DESCRIPTION:                                                               " term-ch))
        (insert (concat "*                                                                            " term-ch))
        (insert (concat "*                                                                            " term-ch))
        (insert "*                                                                            *\n")
        (insert (concat "* PARAMETERS:                                                                " term-ch))
	(if args
	  (make-irit-c-func-ins-params args term-ch)
	  (insert (concat "*   None                                                                     " term-ch)))
        (insert "*                                                                            *\n")
        (insert (concat "* RETURN VALUE:                                                              " term-ch))
	(insert (concat "*   " ret-val-insert))
	(if (string-match "void" ret-val-insert)
	  (insert-char ?  (- 73 (length ret-val-insert)))
	  (if (string-match "NoValue" ret-val-insert)
	    (insert-char ?  (- 73 (length ret-val-insert)))
	    (progn
	      (insert-char ?: 1)
	      (insert-char ?  (- 72 (length ret-val-insert))))))
	(insert term-ch)
	(if (not (string-match "static" (substring func-proto 0 match1)))
	  (progn
	    (insert "*                                                                            *\n")
	    (insert (concat "* SEE ALSO:                                                                  " term-ch))
	    (insert (concat "*                                                                            " term-ch))
	    (insert "*                                                                            *\n")
	    (insert (concat "* KEYWORDS:                                                                  " term-ch))
	    (insert (concat "*   " func-name))
	    (insert-char ?  (- 73 (length func-name)))
	    (insert term-ch)))
        (insert "*****************************************************************************/\n")
	(insert (concat func-proto ""))))
    (message "Should be invoked in C-mode only.")))

(provide 'irit)
