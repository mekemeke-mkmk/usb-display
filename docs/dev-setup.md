# Development Setup

## Why GitHub looked unchanged

Your repository can look unchanged on GitHub when commits exist only in local `main` and are not pushed to `origin/main`.

Current blocker from this environment:

- `git push` fails with: `could not read Username for 'https://github.com': Device not configured`
- This means local GitHub credentials are not available in the current terminal context.

## Verify local progress

```sh
git status --short --branch
git log --oneline --decorate -n 5
```

## Push from your machine

Run in repository root:

```sh
git push origin main
```

If asked, complete GitHub authentication in your terminal.

## Recommended HTTPS auth setup

If `git push` keeps asking or fails on username/password, set up GitHub CLI auth once:

```sh
gh auth login
gh auth setup-git
git push origin main
```

For macOS credential storage with HTTPS remotes:

```sh
git config --global credential.helper osxkeychain
```

## Optional: switch to SSH remote

```sh
git remote set-url origin git@github.com:mekemeke-mkmk/usb-display.git
git push origin main
```
