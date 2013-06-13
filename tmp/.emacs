;;; Xemacs backwards compatibility file
(setq user-init-file
      (expand-file-name "init.el"
			(expand-file-name ".xemacs" "~")))
(setq custom-file
      (expand-file-name "custom.el"
			(expand-file-name ".xemacs" "~")))

;;; User custom files
(load-file user-init-file)
(load-file custom-file)
(global-linum-mode t)

;;; Puts deleted region in Kill Ring
(setq delete-active-region 'kill)

;;; Don't allow commands working on region to use inactive region (last region - current point)
(setq mark-even-if-inactive 'nil)

;;; Transparency
 (set-frame-parameter (selected-frame) 'alpha '(100 100))
 (add-to-list 'default-frame-alist '(alpha 100 100))

;;; Auto fill to wrap long lines
(add-hook 'text-mode-hook
	  (lambda ()
	    (auto-fill-mode 1)
	    (setq default-justification 'full)))

;;; Always add some minor  modes
(column-number-mode)
(tool-bar-mode -1)
(global-hl-line-mode t)
(set-scroll-bar-mode 'right)
(global-auto-revert-mode t)

;;; Parsing file on load and save
(setq TeX-parse-self t) ; Enable parse on load.
(setq TeX-auto-save t) ; Enable parse on save.

;;; No auto fill in some latex environments
(defvar my-LaTeX-no-autofill-environments
  '("tikzpicture" "lstlisting" "tabular" "spreadtab")
  "A list of LaTeX environment names in which `auto-fill-mode' should be inhibited.")

(defun my-LaTeX-auto-fill-function ()
  "This function checks whether point is currently inside one of
the LaTeX environments listed in
`my-LaTeX-no-autofill-environments'. If so, it inhibits automatic
filling of the current paragraph."
  (let ((do-auto-fill t)
        (current-environment "")
        (level 0))
    (while (and do-auto-fill (not (string= current-environment "document")))
      (setq level (1+ level)
            current-environment (LaTeX-current-environment level)
            do-auto-fill (not (member current-environment my-LaTeX-no-autofill-environments))))
    (when do-auto-fill
      (do-auto-fill))))

(defun my-LaTeX-setup-auto-fill ()
  "This function turns on auto-fill-mode and sets the function
used to fill a paragraph to `my-LaTeX-auto-fill-function'."
  (auto-fill-mode)
  (setq auto-fill-function 'my-LaTeX-auto-fill-function))

(add-hook 'LaTeX-mode-hook 'my-LaTeX-setup-auto-fill)

;;; Sets window's title to buffer
(setq frame-title-format
      (list (format "%s %%S: %%j " (system-name))
        '(buffer-file-name "%f" (dired-directory dired-directory "%b"))))

;;; ISpell default to American
(setq ispell-dictionary "american")

;; Put autosave files (ie #foo#) and backup files (ie foo~) in ~/.emacs.d/.
(custom-set-variables
 '(auto-save-file-name-transforms '((".*" "~/.emacs.d/backups/\\1" t)))
 '(backup-directory-alist '((".*" . "~/.emacs.d/backups/"))))

;; create the backups dir if necessary, since emacs won't.
(make-directory "~/.emacs.d/backups/" t)

;;delete old save and backup files
(message "Deleting old backup files...")
(let ((week (* 60 60 24 7))
      (current (float-time (current-time))))
  (dolist (file (directory-files "~/.emacs.d/backups/" t))
    (when (and (backup-file-name-p file)
    	       (> (- current (float-time (nth 6 (file-attributes file))))
    		  week))
      (message "%s\n" file)
      (delete-file file))))

;; setup files ending in “.str” to open in c-mode
(add-to-list 'auto-mode-alist '("\\.str\\'" . c-mode))

;; Custom visual parameters
(setq-default c-basic-offset 4)
(setq-default indicate-buffer-boundaries 'left)
(setq-default show-trailing-whitespace 't)
;;(setq visible-bell 't)

;; Open several file side by side instead of on top of other
(setq split-height-threshold nil)
(setq split-width-threshold 0)
