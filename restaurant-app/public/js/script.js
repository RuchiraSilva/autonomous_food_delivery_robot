document.addEventListener('DOMContentLoaded', () => {
    const socket = io();
    let menu = [];
    let cart = {};

    socket.on('initialMenu', (menuData) => {
        menu = menuData;
        renderMenu();
    });

    function renderMenu() {
        const container = document.getElementById('foodItemsContainer');
        container.innerHTML = '';

        menu.forEach(item => {
            const div = document.createElement('div');
            div.className = 'menu-item';
            div.innerHTML = `
            <h3 style="font-size:15px">${item.name} - Rs. ${item.price}</h3>
            <div class="quantity-container" style="display: flex; align-items: center; gap: 5px;">
                <button class="decrease" style="width: 30px; height: 10px;background-color:transparent; color: black; font-size: 20px; cursor: pointer;display: flex;align-items: center;margin-top:10px;justify-content: center;" onclick="changeQuantity(this, -1)">−</button>
                <input id="quantityInput" class="quantity-input" type="number" min="0" data-id="${item.id}" value="0" style="width: 50px; text-align: center; font-size: 16px; height: 12px;margin-top:10px;">
                <button class="increase" style="width: 30px; height: 10px;background-color:transparent; color: black; font-size: 20px; cursor: pointer;display: flex;align-items: center;margin-top:10px;justify-content: center; margin-left:-10px" onclick="changeQuantity(this, 1)">+</button>
            </div>
            <div style="display: flex; justify-content: flex-end;">
                <button style="color:black; background-color: white; border: 1px solid black; text-align: center;display: flex; justify-content: center; align-items: center; padding: 5px;margin-top:-45px;" onclick="addToCart(${item.id})">
                    <img id="addToCartImage" src="images/add-to-cart.png" height="20px"> Add to Cart
                </button>
            </div>
        `;
            container.appendChild(div);
        });
    }

    window.addToCart = function(id) {
        const input = document.querySelector(`.quantity-input[data-id="${id}"]`);
        const quantity = parseInt(input.value);

        if (!input || isNaN(quantity) || quantity <= 0) {
            alert('Please enter a valid quantity.');
            return;
        }

        const menuItem = menu.find(item => item.id === id);
        if (!menuItem) {
            console.error(`Menu item with ID ${id} not found.`);
            return;
        }

        cart[menuItem.name] = (cart[menuItem.name] || 0) + quantity;

        console.log(`Added ${quantity} of ${menuItem.name} to cart. Cart:`, cart);

        updateCart();

        input.value = 0;
    };

    function updateCart() {
        const cartItems = document.getElementById('cartItems');
        const totalPriceElement = document.getElementById('totalPrice');
        let totalPrice = 0;

        cartItems.innerHTML = '';

        Object.entries(cart).forEach(([name, quantity]) => {
            const menuItem = menu.find(item => item.name === name);
            if (!menuItem) {
                console.error(`Menu item "${name}" not found in menu.`);
                return;
            }

            const unitPrice = menuItem.price;
            const totalPriceForItem = unitPrice * quantity;
            totalPrice += totalPriceForItem;

            const row = document.createElement('tr');

            const itemNameCell = document.createElement('td');
            itemNameCell.textContent = name;
            row.appendChild(itemNameCell);

            const unitPriceCell = document.createElement('td');
            unitPriceCell.textContent = unitPrice.toFixed(2);
            row.appendChild(unitPriceCell);

            const quantityCell = document.createElement('td');
            quantityCell.textContent = quantity;
            row.appendChild(quantityCell);

            const totalPriceCell = document.createElement('td');
            totalPriceCell.textContent = totalPriceForItem.toFixed(2);
            row.appendChild(totalPriceCell);

            const actionCell = document.createElement('td');
            const removeButton = document.createElement('button');
            removeButton.textContent = 'Remove';
            removeButton.className = 'remove-button';
            removeButton.onclick = () => removeFromCart(name);
            actionCell.appendChild(removeButton);
            row.appendChild(actionCell);

            cartItems.appendChild(row);
        });


        totalPriceElement.textContent = totalPrice.toFixed(2);


        console.log('Cart updated. Total Price:', totalPrice);
    }


    function removeFromCart(name) {
        delete cart[name];
        updateCart();
    }


    const viewCartButton = document.getElementById('viewCartButton');
    const cartSection = document.getElementById('cartSection');

    if (!viewCartButton || !cartSection) {
        console.error('View Cart Button or Cart Section not found!');
    } else {
        console.log('View Cart Button and Cart Section found.');
        viewCartButton.addEventListener('click', () => {
            console.log('View Cart Button Clicked');
            cartSection.classList.toggle('active');
            console.log('Cart Visibility:', cartSection.classList.contains('active'));
            viewCartButton.textContent = cartSection.classList.contains('active') ? '🛒' : '🛒View Cart';
        });
    }

    document.getElementById('orderForm').addEventListener('submit', (e) => {
        e.preventDefault();

        if (Object.keys(cart).length === 0) {
            alert('Please add at least one food item to your cart.');
            return;
        }

        const tableNumber = document.getElementById('tableNumber').value;
        const email = document.getElementById('email').value;
        const totalPrice = parseFloat(document.getElementById('totalPrice').textContent);

        const orderDetails = Object.entries(cart)
            .map(([name, quantity]) => `${name} x${quantity}`)
            .join(', ');

        const popupMessage = `
            <div style="font-family: Arial, sans-serif; color: #333;">
                <h2>Thank you for your order!</h2>
                <p><strong>Table Number:</strong> ${tableNumber}</p>
                <p><strong>Ordered Items:</strong></p>
                <ul>
                    ${Object.entries(cart)
                        .map(([name, quantity]) => `<li>${name} x${quantity}</li>`)
                        .join('')}
                </ul>
                <p><strong>Total Price:</strong> Rs. ${totalPrice.toFixed(2)}</p>
            </div>
        `;

        document.getElementById('popupMessage').innerHTML = popupMessage.trim();
        document.getElementById('popup').style.display = 'flex';

        const closePopupButton = document.getElementById('closePopup');
        closePopupButton.onclick = () => {
            document.getElementById('popup').style.display = 'none';

            fetch('/submit-order', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify({
                    tableNumber,
                    items: cart,
                    totalPrice
                })
            })
            .then(response => {
                if (response.redirected) {
                    window.location.href = response.url;
                }
            })
            .catch(error => console.error('Error placing order:', error));

            if (email) {
                fetch('/send-bill', {
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/json'
                    },
                    body: JSON.stringify({
                        email,
                        orderDetails: popupMessage.trim()
                    })
                })
                .then(response => response.text())
                .then(message => {
                    console.log(message);
                })
                .catch(error => console.error('Error sending email:', error));
            }

            cart = {};
            updateCart();
        };
    });
});

function changeQuantity(button, amount) {
    let input = button.closest('.quantity-container').querySelector('.quantity-input');
    let value = parseInt(input.value) || 0;
    value = Math.max(0, value + amount);
    input.value = value;
}
