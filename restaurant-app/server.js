const express = require('express');
const bodyParser = require('body-parser');
const sqlite3 = require('sqlite3').verbose();
const path = require('path');
const http = require('http');
const socketIo = require('socket.io');
const nodemailer = require('nodemailer');
const { google } = require('googleapis');

const app = express();
const PORT = 3000;

const server = http.createServer(app);
const io = socketIo(server);

app.use(bodyParser.urlencoded({ extended: true }));
app.use(express.static(path.join(__dirname, 'public')));
app.use('/admin', express.static(path.join(__dirname, 'admin')));
app.use(bodyParser.json());
app.use(express.json());

const CLIENT_ID = 'client_id';
const CLIENT_SECRET = 'client_secret';
const REFRESH_TOKEN = 'refresh_token';
const REDIRECT_URI = 'redirect_uri';

const oauth2Client = new google.auth.OAuth2(CLIENT_ID, CLIENT_SECRET, REDIRECT_URI);
oauth2Client.setCredentials({ refresh_token: REFRESH_TOKEN });


const db = new sqlite3.Database('./db.sqlite');
db.serialize(() => {

    db.run(`CREATE TABLE IF NOT EXISTS menu (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        name TEXT NOT NULL,
        price REAL NOT NULL
    )`);

    db.run(`CREATE TABLE IF NOT EXISTS orders (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        order_datetime TEXT DEFAULT (datetime('now', 'localtime')),
        table_number TEXT NOT NULL,
        items TEXT NOT NULL, -- JSON string of {food_item: quantity}
        total_price REAL NOT NULL,
        status TEXT DEFAULT 'pending'
    )`);
});

app.get('/menu', (req, res) => {
    db.all("SELECT * FROM menu", [], (err, rows) => {
        if (err) {
            return res.status(500).send('Error fetching menu.');
        }
        res.json(rows);
    });
});

app.post('/admin/add-food-item', (req, res) => {
    const { name, price } = req.body;
    db.run("INSERT INTO menu (name, price) VALUES (?, ?)", [name, price], function(err) {
        if (err) {
            return res.status(500).send('Error adding food item.');
        }
        res.send('Food item added successfully.');

        db.get("SELECT * FROM menu WHERE id = ?", [this.lastID], (err, row) => {
            if (!err) {
                io.emit('newFoodItem', row);
            }
        });
    });
});

app.post('/submit-order', (req, res) => {
    const { tableNumber, items, totalPrice } = req.body;

    const itemsJson = JSON.stringify(items);

    const stmt = db.prepare("INSERT INTO orders (table_number, items, total_price) VALUES (?, ?, ?)");
    stmt.run([tableNumber, itemsJson, totalPrice], function(err) {
        if (err) {
            return res.status(500).send('Error placing order.');
        }
        res.redirect('/');

        db.get("SELECT * FROM orders WHERE id = ?", [this.lastID], (err, row) => {
            if (!err) {
                io.emit('newOrder', row);
            }
        });
    });
    stmt.finalize();
});

app.get('/admin/orders', (req, res) => {
    const sort = req.query.sort || 'desc';
    const orderClause = sort === 'asc' ? 'ASC' : 'DESC';

    db.all(`SELECT * FROM orders ORDER BY id ${orderClause}`, [], (err, rows) => {
        if (err) {
            return res.status(500).send('Error fetching orders.');
        }
        console.log('Fetched orders:', rows);
        res.json(rows);
    });
});


app.post('/admin/update-status', (req, res) => {
    const { id, status } = req.body;
    console.log(`Updating order ${id} to status: ${status}`);

    db.run("UPDATE orders SET status = ? WHERE id = ?", [status, id], function(err) {
        if (err) {
            return res.status(500).send('Error updating status.');
        }
        res.send('Status updated successfully.');

        db.get("SELECT * FROM orders WHERE id = ?", [id], (err, row) => {
            if (!err) {
                io.emit('orderUpdated', row);
            }
        });
    });
});

app.post('/admin/delete-order', (req, res) => {
    const { id } = req.body;
    db.run("DELETE FROM orders WHERE id = ?", [id], function(err) {
        if (err) {
            return res.status(500).send('Error deleting order.');
        }
        res.send('Order deleted successfully.');

        io.emit('orderDeleted', id);
    });
});

app.post('/admin/delete-food-item', (req, res) => {
    const { id } = req.body;
    db.run("DELETE FROM menu WHERE id = ?", [id], function(err) {
        if (err) {
            return res.status(500).send('Error deleting food item.');
        }
        res.send('Food item deleted successfully.');

        io.emit('foodItemDeleted', id);
    });
});

app.post('/send-bill', async (req, res) => {
    const { email, orderDetails } = req.body;

    try {
        const accessToken = await oauth2Client.getAccessToken();

        const transporter = nodemailer.createTransport({
            service: 'gmail',
            auth: {
                type: 'OAuth2',
                user: 'restaurant_gmail_address',
                clientId: CLIENT_ID,
                clientSecret: CLIENT_SECRET,
                refreshToken: REFRESH_TOKEN,
                accessToken: accessToken.token,
            },
        });

        const mailOptions = {
            from: 'restaurant_gmail_address',
            to: email,
            subject: 'Your Restaurant Order Receipt',
            html: `
                <h1>Thank you for your order!</h1>
                <p><strong>Order Details:</strong></p>
                <pre>${orderDetails}</pre>
            `,
        };

        await transporter.sendMail(mailOptions);
        res.send('Email sent successfully.');
    } catch (error) {
        console.error('Error sending email:', error);
        res.status(500).send('Error sending email.');
    }
});

io.on('connection', (socket) => {
    console.log('A client connected');

    db.all("SELECT * FROM menu", [], (err, rows) => {
        if (!err) {
            socket.emit('initialMenu', rows);
        }
    });

    db.all("SELECT * FROM orders", [], (err, rows) => {
        if (!err) {
            socket.emit('initialOrders', rows);
        }
    });

    socket.on('disconnect', () => {
        console.log('A client disconnected');
    });
});

server.listen(PORT, () => {
    console.log(`Server running at http://localhost:${PORT}`);
});