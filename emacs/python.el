; 
; python.el - Definitions of PYTHON mode for emacs editor.
; 
; Author:	Gershon Elber
; 		Computer Science Dept.
; 		Technion, Israel Institute of technology
; Date:	Tue May 14 1991
; Copyright (c) 1991, Gershon Elber
;
; This file defines an environment to run edit and execute PYTHON programs.
; Such a program should have a '.py' extension in order it to be in
; python-mode major mode. Several new functions are provided to communicate
; between the editted file and the solid modeller:
;
; 1. send-line-to-python - sends a single line to the solid modeller for
;    execution. A line is defined from current position to the next
;    semicolon ';'. If however several commands exists on the same line
;    they will all be send as one line.
;    Bounded to Meta-E by default.
; 2. send-region-to-python - sends the region from the current mark (mark-marker)
;    to current position (point-marker) to the solid modeller. This function
;    is convenient for sending a large block of commands.
;    Bounded to Meta-R by default.
; 3. send-mini-buffer-to-python - sends a line retrieved via the mini buffer to
;    the solid modeller for execution. The line is appended with a new line
;    and is echoed to the python buffer if python-echo-program.
;    Bounded to Meta-S by default.
;
; Both functions checks for existance of a buffer named python
; and a process named "python" hooked to it, and will restart a new process
; or buffer if none exists. The program to execute as process "python" is
; defined by the python-program constant below.
;
; Mod: Martin Glanvill 15/8/95 (emacs 19.29 & xemacs 19.12)
; Place this file in emacs site-lisp dir and Add: 
;       (require 'python)
;       (add-hook 'python-mode-hook 'turn-on-font-lock t)
;       (add-hook 'python-mode-hook 'outline-minor-mode t)
;
; to your ~/.emacs to get syntax highlighting


(defvar python-program "python.exe"
  "*The executable to run for python buffer.")

(defvar python-args "-i"
  "*Optional arguments of python executable.")

(defvar python-echo-program t
  "*Control echo of executed commands to python buffer.")

(defvar python-mode-map nil "")
(if python-mode-map
    ()
  (setq python-mode-map (make-sparse-keymap))
  (define-key python-mode-map "\M-s" 'send-mini-buffer-to-python)
  (define-key python-mode-map "\M-e" 'send-line-to-python)
  (define-key python-mode-map "\M-r" 'send-region-to-python))

;;;
;;; Add fontlock hook
;;;
(add-hook 'python-mode-hook
 '(lambda () (make-local-variable 'font-lock-defaults)
               (setq font-lock-defaults '(python-font-lock-keywords nil nil))))

;;;
;;; Boolean function to test if running xemacs or lucid emacs
;;;
(defun python-xemacs ()
  (or (string-match "Lucid"  emacs-version)
      (string-match "XEmacs" emacs-version)))

;;;
;;; Define the python-mode
;;;
;;; Instantiate the menu & syntax table (with parenthesis matching).
;;;
(defun python-mode ()
  "Major mode for editing and executing PYTHON files.

see send-line-to-python and send-region-to-python for more."
  (interactive)
  (kill-all-local-variables)
  (use-local-map python-mode-map)
  (setq major-mode 'python-mode)
  (setq mode-name "Python")
; (python-add-python-menu)
  (make-local-variable 'fill-column)
  (setq fill-column 75)
  (setq blink-matching-paren t)
;  (if python-mode-syntax-table
;      (set-syntax-table python-mode-syntax-table)
;    (python-create-syntax-table))
  (run-hooks 'python-mode-hook))

;;;
;;; Internal function send the string to PYTHON in small chunks - Otherwise
;;; Emacs-PYTHON IPC hangs for some unknown reason.
;;;
(defun send-string-to-python (str)
  (if (< (length str) 100)
    (let* ((crnt-buffer (buffer-name)))
      (process-send-string "python" str)
      (switch-to-buffer-other-window (get-buffer "python"))
      (recenter -2)
      (switch-to-buffer-other-window (get-buffer crnt-buffer)))
    (progn
      (process-send-string "python" (substring str 0 100))
      (send-string-to-python (substring str 100)))))

;;;
;;; Define send-min-buffer-to-python - send one line prompt for at the mini
;;; buffer, to the python buffer.
;;;
(defun send-mini-buffer-to-python ()
  "Sends one line of code from mini-buffer to the PYTHON program.

The PYTHON solid modeller buffer name is python and the 
process name is 'python'. If none exists, a new one is created.

The name of the python program program to execute is stored in python-program
and may be changed."
  (interactive)
  (if (equal major-mode 'python-mode)
    (progn
      (make-python-buffer)     ; In case we should start a new one.
      (let* ((crnt-buffer (buffer-name))
	     (string-copy (read-from-minibuffer "Python> ")))
	(switch-to-buffer-other-window (get-buffer "python"))
	(end-of-buffer)
	(if python-echo-program
	  (progn
	    (insert string-copy)
	    (insert "\n")))
	;(process-send-string "python" string-copy)
	(send-string-to-python string-copy)
	(process-send-string "python" "\n")
	(switch-to-buffer-other-window (get-buffer crnt-buffer))))
    (message "Should be invoked in python-mode only.")))

;;;
;;; Define send-line-to-python - send from current cursor position to next
;;; semicolon detected.
;;;
(defun send-line-to-python ()
  "Sends one line of code from current buffer to the PYTHON program.

Use to execute a line in the PYTHON solid modeller. A line is anything
that is terminated by a semicolon, but is at least one line of text so
multiple commands per line (with several semicolons) are still
considered a single line.

The PYTHON solid modeller buffer name is python and the 
process name is 'python'. If none exists, a new one is created.

The name of the python program program to execute is stored in python-program
and may be changed."
  (interactive)
  (if (equal major-mode 'python-mode)
    (progn
      (make-python-buffer)        ; In case we should start a new one.
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
	    (switch-to-buffer-other-window (get-buffer "python"))
	    (end-of-buffer)
	    (if python-echo-program
	      (progn
		(insert string-copy)
		(insert "\n")))
	    (set-marker (process-mark (get-process "python")) (point-marker))
	    (switch-to-buffer-other-window (get-buffer crnt-buffer))
	    (send-string-to-python string-copy)
	    ;(process-send-string "python" string-copy)
	    ;(process-send-region "python" start-mark end-max-mark)
	    (goto-char end-max-mark)
	    (if (equal "\n" (buffer-substring (point-marker)
					      (+ 1 (point-marker))))
	      (process-send-string "python" "\n"))  
	    (if (> end-one-mark end-two-mark)
	      (forward-char 1))))))
    (message "Should be invoked in python-mode only.")))

;;;
;;; Define send-region-to-python - send from current cursor position to
;;; current marker.
;;;
(defun send-region-to-python ()
  "Sends a region of code from current buffer to the PYTHON program.

When this function is invoked on an PYTHON file it send the region from current
point to current mark to the python solid modeller.

The PYTHON solid modeller buffer name is python and the
process name is 'python'. If none exists, a new one is created.

The name of the python program program to execute is stored in python-program
and may be changed."
  (interactive)
  (if (equal major-mode 'python-mode)
    (progn
      (make-python-buffer)     ; In case we should start a new one.
      (copy-region-as-kill (mark-marker) (point-marker))
      (let ((crnt-buffer (buffer-name)))
	(switch-to-buffer-other-window (get-buffer "python"))
	(end-of-buffer)
	(if python-echo-program
	  (yank))
	(set-marker (process-mark (get-process "python")) (point-marker))
	(switch-to-buffer-other-window (get-buffer crnt-buffer))
	;(process-send-region "python" (mark-marker) (point-marker))))
	(send-string-to-python (buffer-substring (mark-marker) (point-marker)))))
    (message "Should be invoked in python-mode only.")))

;;;
;;; Switch to "python" buffer if exists. If not, creates one and
;;; execute the program defined by python-program.
;;;
(defun make-python-buffer ()
  "Switch to python buffer or create one if none exists"
  (interactive)
  (if (not (get-process "python"))
    (start-process "python" "python" python-program python-args)))

;;;
;;; Autoload python-mode on any file with py extension. 
;;;
(setq auto-mode-alist (append '(("\\.py$" . python-mode))
			      auto-mode-alist))

(provide 'python)

