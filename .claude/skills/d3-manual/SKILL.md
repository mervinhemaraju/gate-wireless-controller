---
name: d3-manual
description: Look up CP80 terminal pinouts, status LED flash codes, or any other detail in the Centurion D3/D5 installation manual, OCRing the scanned PDF once and caching the text. Invoke with /d3-manual <what you need to find>.
allowed-tools: Read, Grep, Glob, WebFetch(domain:www.centsys.co.za), Bash(pdftoppm:*), Bash(pdftotext:*), Bash(tesseract:*), Bash(curl:*), Bash(python3:*), Bash(ls:*), Bash(which:*), Bash(mkdir:*), Write(docs/**)
---

# D3 Manual Lookup: $ARGUMENTS

Find `$ARGUMENTS` in the Centurion D3/D5 installation manual.

**The manual is a scanned PDF: the pages are images, not text.** A plain text
extraction returns nothing or near-nothing. That dead end has been hit before;
do not repeat it. Follow the pipeline below.

Manual URL (also in `.claude/CLAUDE.md`):
`https://www.centsys.co.za/upload/0_07_A_0115_%20D3D5%20installation%20manual%2022072013-BM-for%20web.pdf`

## 1. Check the Cache First

Look under `docs/d3-manual/` for already-extracted text. If the answer is
there, use it and skip straight to reporting. Extraction is slow; do it once.

## 2. Fetch

If the PDF is not already cached in the scratchpad or `docs/d3-manual/`,
download it there. Keep the PDF itself out of git: cache the extracted **text**
in `docs/d3-manual/`, not the binary.

## 3. Confirm It Is Scanned

Run `pdftotext` on a couple of pages first. If it yields real text, the job is
done cheaply and the rest of this pipeline is unnecessary. Expect it to come
back empty.

## 4. Render and OCR

Preferred path, poppler plus tesseract:

```bash
pdftoppm -r 300 -png -f <first> -l <last> manual.pdf page
tesseract page-01.png page-01 --psm 6
```

- 300 dpi minimum. Lower resolutions mangle the terminal labels, which is
  exactly the detail being looked up
- PyMuPDF is an acceptable alternative if poppler is unavailable
- If neither toolchain is installed, stop and tell the user which one to
  install rather than installing it yourself

Only OCR the pages likely to hold the answer. The wiring and terminal sections
plus the LED code tables are a small slice of the document; rendering all of it
at 300 dpi is wasteful.

## 5. Cache

Write the extracted text to `docs/d3-manual/pXX.txt` (or a section-named file),
so the next lookup is a `grep`.

## 6. Report

- Quote the relevant passage verbatim, with the page number
- **Flag OCR uncertainty explicitly.** OCR confuses `0`/`O`, `1`/`l`, `8`/`B`,
  and drops decimal points. A misread terminal number or voltage gets wired
  into a live board, so when a character is doubtful, say so and tell the user
  to confirm against the PDF page directly
- If the manual does not answer the question, say that plainly. Do not fill
  the gap from general knowledge of gate motors: this is wiring a specific
  board, and a plausible guess is more dangerous than an admitted gap
