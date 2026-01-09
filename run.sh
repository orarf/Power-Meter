set -e

echo "Running..."

EXECUTABLE="./build/main"
TB_TOKEN="pEgujBQots9Mq3pQY6lQ"

if [ -f "$EXECUTABLE" ]; then
    chmod +x "$EXECUTABLE"
    "$EXECUTABLE" "$TB_TOKEN"
else
    echo "Error: Executable not found!"
    exit 1
fi