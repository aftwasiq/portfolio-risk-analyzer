FROM python:3.11-slim

# Install system build tools and network utilities
RUN apt-get update && apt-get install -y \
    build-essential \
    curl \
    ca-certificates \
    dnsutils \
    iputils-ping \
    && rm -rf /var/lib/apt/lists/*

# Update CA certificates
RUN update-ca-certificates

# Set working directory
WORKDIR /app

# Copy requirements first for caching
COPY requirements.txt .

# Upgrade pip, setuptools, wheel BEFORE installing requirements
RUN python -m pip install --upgrade pip setuptools wheel

# Install Python dependencies
RUN pip install --no-cache-dir -r requirements.txt

# Copy all app files
COPY . .

# Create upload directory and set permissions
RUN mkdir -p /app/uploads && chmod 777 /app/uploads

# Expose port
EXPOSE 5000

# Default command
CMD ["python", "web/app.py"]
