<?php
header('Content-Type: text/html; charset=UTF-8');

$query = $_SERVER['QUERY_STRING'] ?? '';
$method = $_SERVER['REQUEST_METHOD'] ?? 'UNKNOWN';
$script = $_SERVER['SCRIPT_NAME'] ?? '';
$pathinfo = $_SERVER['PATH_INFO'] ?? '';

function safe($value) {
    return htmlspecialchars($value, ENT_QUOTES | ENT_SUBSTITUTE, 'UTF-8');
}

echo '<!DOCTYPE html>';
echo '<html lang="en">';
echo '<head>';
echo '  <meta charset="UTF-8">';
echo '  <meta name="viewport" content="width=device-width, initial-scale=1.0">';
echo '  <title>PHP CGI Test</title>';
echo '  <style>';
echo '    body { margin: 0; font-family: "Inter", system-ui, sans-serif; background: linear-gradient(135deg, #1b262c, #0f4c75); color: #e9ecef; }';
echo '    .page { min-height: 100vh; display: flex; align-items: center; justify-content: center; padding: 2rem; }';
echo '    .card { width: min(860px, 100%); background: rgba(255,255,255,0.1); border: 1px solid rgba(255,255,255,0.15); border-radius: 24px; box-shadow: 0 32px 80px rgba(0,0,0,0.3); backdrop-filter: blur(16px); padding: 2rem; }';
echo '    h1 { margin: 0 0 1rem; font-size: clamp(2rem, 2.5vw, 3rem); letter-spacing: 0.04em; }';
echo '    .meta { display: grid; gap: 1rem; }';
echo '    .meta p { margin: 0; padding: 1rem 1.2rem; border-radius: 18px; background: rgba(255,255,255,0.08); border: 1px solid rgba(255,255,255,0.12); }';
echo '    .meta span { display: block; color: #a5b7c7; font-size: 0.95rem; margin-bottom: 0.35rem; }';
echo '    .footer { margin-top: 1.7rem; color: #d8e2dc; font-size: 0.95rem; }';
echo '  </style>';
echo '</head>';
echo '<body>';
echo '  <div class="page">';
echo '    <div class="card">';
echo '      <h1>PHP CGI Test</h1>';
echo '      <div class="meta">';
echo '        <p><span>Method</span>' . safe($method) . '</p>';
echo '        <p><span>Query string</span>' . safe($query) . '</p>';
echo '        <p><span>Script name</span>' . safe($script) . '</p>';
echo '        <p><span>Path info</span>' . safe($pathinfo) . '</p>';
echo '      </div>';
echo '      <div class="footer">Beautiful PHP CGI output for Webserv testing.</div>';
echo '    </div>';
echo '  </div>';
echo '</body>';
echo '</html>';
