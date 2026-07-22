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

Look under `docs/reference/` for already-extracted text (files named
`d3-manual-*`). If the answer is there, use it and skip straight to reporting.
Extraction is slow; do it once.

## 2. Fetch

If the PDF is not already cached in the scratchpad, download it there. Keep the
PDF itself out of git: cache the extracted **text** in `docs/reference/`, not
the binary.

## 3. Confirm It Is Scanned

Run `pdftotext` on a couple of pages first. If it yields real text, the job is
done cheaply and the rest of this pipeline is unnecessary. Expect it to come
back empty.

## 4. Read the Pages

Two paths. Prefer the visual read: it needs only poppler and avoids the
character-confusion that plagues OCR of the terminal labels.

**Preferred: visual read via the Read tool (poppler only, no tesseract).**
Point the Read tool at the PDF with a `pages` range. The harness renders the
pages (via poppler's `pdftoppm`) and presents them as images, which are read
directly. Find the section with the Table of Contents first, then read only the
pages likely to hold the answer (max 20 pages per Read call). Still flag any
character that looks doubtful and say to confirm against the PDF page.

**Fallback: OCR with tesseract**, only if the Read tool cannot render the PDF:

```bash
pdftoppm -r 300 -png -f <first> -l <last> manual.pdf page
tesseract page-01.png page-01 --psm 6
```

- 300 dpi minimum. Lower resolutions mangle the terminal labels, which is
  exactly the detail being looked up
- PyMuPDF is an acceptable alternative if poppler is unavailable

Both paths need poppler. If it is not installed, stop and tell the user to
install it (`brew install poppler`) rather than installing it yourself. Either
way, only process the pages likely to hold the answer: the wiring and terminal
sections plus the LED code tables are a small slice of the document, and
rendering all 48 pages is wasteful.

## 5. Cache

Write the extracted text to `docs/reference/`, following
`.claude/rules/docs-conventions.md`: kebab-case names that mirror the source,
`d3-manual-pXX-<topic>.md` (for example `d3-manual-p31-p41-cp80-terminals.md`),
each with the standard doc header. Add an index line for every file to the
`reference/` section of `docs/README.md`, or the cache is undiscoverable. This
way the next lookup is a `grep` over `docs/reference/`.

## 6. Report

- Quote the relevant passage verbatim, with the page number
- **Flag OCR uncertainty explicitly.** OCR confuses `0`/`O`, `1`/`l`, `8`/`B`,
  and drops decimal points. A misread terminal number or voltage gets wired
  into a live board, so when a character is doubtful, say so and tell the user
  to confirm against the PDF page directly
- If the manual does not answer the question, say that plainly. Do not fill
  the gap from general knowledge of gate motors: this is wiring a specific
  board, and a plausible guess is more dangerous than an admitted gap
