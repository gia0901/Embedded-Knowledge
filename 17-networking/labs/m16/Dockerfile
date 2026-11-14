FROM python:3.12-slim
ENV PYTHONDONTWRITEBYTECODE=1 PYTHONUNBUFFERED=1
RUN pip install --no-cache-dir pyjwt==2.9.0 cryptography==43.0.3
WORKDIR /app
COPY app/ /app/
USER 10001
