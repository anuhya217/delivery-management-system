CREATE DATABASE IF NOT EXISTS delivery_management;

USE delivery_management;

-- DSPs Table
CREATE TABLE IF NOT EXISTS dsps (
    id INT AUTO_INCREMENT PRIMARY KEY,
    name VARCHAR(100) NOT NULL,
    contact VARCHAR(100),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Drivers Table
CREATE TABLE IF NOT EXISTS drivers (
    id INT AUTO_INCREMENT PRIMARY KEY,
    name VARCHAR(100) NOT NULL,
    phone VARCHAR(20),
    status VARCHAR(30) DEFAULT 'active',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Orders Table
CREATE TABLE IF NOT EXISTS orders (
    id INT AUTO_INCREMENT PRIMARY KEY,
    dsp_id INT,
    driver_id INT,
    pickup_location VARCHAR(255) NOT NULL,
    delivery_location VARCHAR(255) NOT NULL,
    status VARCHAR(30) DEFAULT 'pending',
    sla_deadline DATETIME,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,

    FOREIGN KEY (dsp_id) REFERENCES dsps(id),
    FOREIGN KEY (driver_id) REFERENCES drivers(id)
);

-- Sample DSP
INSERT INTO dsps (name, contact)
VALUES ('Delivery Partner 1', 'contact@example.com');

-- Sample Driver
INSERT INTO drivers (name, phone, status)
VALUES ('Driver 1', '9876543210', 'active');

-- Sample Order
INSERT INTO orders (
    dsp_id,
    driver_id,
    pickup_location,
    delivery_location,
    status,
    sla_deadline
)
VALUES (
    1,
    1,
    'Hyderabad',
    'Visakhapatnam',
    'pending',
    '2026-08-30 00:00:00'
);