/**
 * @file       admin.js
 * @author     Ruchira Silva
 * @version    1.0
 * @date       March 2025
 * @brief      Node.js backend for Autonomous Food Delivery Robot.
 *
 * @details    This file contains the backend logic for managing orders, food items,
 *             and communication with the ESP32 microcontroller. It integrates with
 *             an SQLite database and provides RESTful APIs for the web interface.
 *
 * @copyright  Copyright (c) 2025 Ruchira Silva. All rights reserved.
 *
 * Project Links:
 * - GitHub Repository: https://github.com/RuchiraSilva/autonomous_food_delivery_robot
 * - YouTube Demo: https://youtu.be/X6bn6PqycP4?si=IvlOGM1kDrbI3rAY
 *
 * Contact me:
 * - Email: ruchirasilva45@gmail.com
 * - LinkedIn: https://www.linkedin.com/in/ruchirasilva
 * 
 */

document.addEventListener('DOMContentLoaded', () => {
    const socket = io();

    socket.on('initialMenu', (menu) => {
        renderMenu(menu);
    });

    socket.on('initialOrders', (orders) => {
        renderOrders(orders);
    });

    socket.on('newFoodItem', (foodItem) => {
        const menuList = document.querySelector('#menuList');
        const listItem = createMenuItem(foodItem);
        menuList.appendChild(listItem);
    });

    socket.on('foodItemDeleted', (id) => {
        const listItem = document.querySelector(`.menu-item-${id}`);
        if (listItem) {
            listItem.remove();
        }
    });

    socket.on('newOrder', (order) => {
        const tbody = document.querySelector('#ordersTable tbody');
        const row = createOrderRow(order);
        row.classList.add('new-order');
        tbody.prepend(row);
    });

    socket.on('orderUpdated', (updatedOrder) => {
        const statusCell = document.querySelector(`.status-${updatedOrder.id}`);
        const actionCell = statusCell.nextElementSibling;

        if (statusCell) {
            statusCell.textContent = updatedOrder.status;
        }

        if (actionCell) {
            actionCell.innerHTML = updatedOrder.status === 'completed' 
                ? '<span style="color: green;">Completed</span>' 
                : `<button onclick="updateStatus(${updatedOrder.id}, 'completed')">Mark as Completed</button>`;
        }
    });

    socket.on('orderDeleted', (id) => {
        const row = document.querySelector(`.status-${id}`).parentElement;
        if (row) {
            row.remove();
        }
    });

    document.getElementById('addFoodForm').addEventListener('submit', (e) => {
        e.preventDefault();
        const foodName = document.getElementById('foodName').value;
        const foodPrice = parseFloat(document.getElementById('foodPrice').value);
        fetch('/admin/add-food-item', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({ name: foodName, price: foodPrice })
        })
        .then(response => response.text())
        .then(message => {
            alert(message);
            document.getElementById('addFoodForm').reset();
        })
        .catch(error => console.error('Error adding food item:', error));
    });

    document.getElementById('returnHomeButton').addEventListener('click', () => {
        fetch(`http://<ip_of_the_robot>/move?table=5`)
            .then(response => response.text())
            .then(message => {
                console.log(message);
                alert('Robot returning home...');
            })
            .catch(error => console.error('Error sending command to ESP32:', error));
    });
});

function renderMenu(menu) {
    const menuList = document.querySelector('#menuList');
    menuList.innerHTML = '';

    menu.forEach(item => {
        const listItem = createMenuItem(item);
        menuList.appendChild(listItem);
    });
}

function createMenuItem(item) {
    const listItem = document.createElement('li');
    listItem.className = `menu-item-${item.id}`;
    listItem.innerHTML = `
        ${item.name} - Rs. ${item.price}
        <button onclick="deleteFoodItem(${item.id})" style="margin-left: 10px;">Delete</button><br><br>
    `;
    return listItem;
}

function deleteFoodItem(id) {
    fetch('/admin/delete-food-item', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({ id })
    })
    .then(response => response.text())
    .then(message => {
        alert(message);
    })
    .catch(error => console.error('Error deleting food item:', error));
}

function renderOrders(orders) {
    const tbody = document.querySelector('#ordersTable tbody');
    tbody.innerHTML = '';

    orders.forEach(order => {
        const row = createOrderRow(order);
        tbody.prepend(row);
    });
}

function createOrderRow(order) {
    const items = JSON.parse(order.items);
    const itemsText = Object.entries(items).map(([name, quantity]) => `${name} x${quantity}`).join(', ');
    const row = document.createElement('tr');
    row.innerHTML = `
        <td>${order.id}</td>
        <td>${order.order_datetime}</td>
        <td class="table-number-${order.id}">${order.table_number}</td>
        <td>${itemsText}</td>
        <td>Rs. ${order.total_price.toFixed(2)}</td>
        <td class="status-${order.id}">${order.status}</td>
        <td>
            ${order.status === 'completed' ? 
                '<span style="color: green;">Completed</span>' : 
                `<button onclick="updateStatus(${order.id}, 'completed')">Mark as Completed</button>`}
            ${order.status === 'completed' ? 
                `<button onclick="deleteOrder(${order.id})" style="margin-left: 10px;">Delete</button>` : ''}
        </td>
    `;
    return row;
}

function updateStatus(id, status) {
    const tableNumber = document.querySelector(`.table-number-${id}`).textContent;

    fetch(`http://<ip_of_the_robot>/move?table=${tableNumber}`)
        .then(response => response.text())
        .then(message => {
            console.log(message);
        })
        .catch(error => console.error('Error sending command to ESP32:', error));

    fetch('/admin/update-status', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({ id, status })
    })
    .then(response => response.text())
    .then(message => {
        alert(message);
    })
    .catch(error => console.error('Error updating status:', error));
}

function deleteOrder(id) {
    fetch('/admin/delete-order', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({ id })
    })
    .then(response => response.text())
    .then(message => {
        alert(message);
    })
    .catch(error => console.error('Error deleting order:', error));
}

let sortOrder = 'desc';
document.getElementById('toggleSort').addEventListener('click', () => {
    sortOrder = sortOrder === 'desc' ? 'asc' : 'desc';
    document.getElementById('toggleSort').textContent = 
        sortOrder === 'desc' ? 'Sort Orders (Oldest First)' : 'Sort Orders (Newest First)';
    
    fetch(`/admin/orders?sort=${sortOrder}`)
        .then(response => response.json())
        .then(orders => {
            renderOrders(orders);
        })
        .catch(error => console.error('Error fetching orders:', error));
});