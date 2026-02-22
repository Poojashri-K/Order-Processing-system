const express = require("express");
const cors = require("cors");
const bodyParser = require("body-parser");
const fs = require("fs-extra");

const app = express();
const PORT = 3000;
const STORE_FILE = "./orders.json";

app.use(cors());
app.use(bodyParser.json());

// ---------------- Helper Functions ----------------
async function readStore() {
  try {
    const data = await fs.readFile(STORE_FILE, "utf-8");
    return JSON.parse(data || "[]");
  } catch (err) {
    console.warn("⚠️ No existing file, creating new store:", err.message);
    return [];
  }
}

async function writeStore(data) {
  try {
    await fs.writeFile(STORE_FILE, JSON.stringify(data, null, 2));
  } catch (err) {
    console.error("💥 Failed to write file:", err.message);
  }
}

// ---------------- ROUTES ----------------

// ✅ Get all orders
app.get("/api/order", async (req, res) => {
  const orders = await readStore();
  res.json(orders);
});

// ✅ Create new order
app.post("/api/order", async (req, res) => {
  try {
    const { customer, items, quantity, priority } = req.body;

    if (!customer || !items || !quantity) {
      console.log("❌ Missing fields:", req.body);
      return res.status(400).json({ error: "Missing fields" });
    }

    const orders = await readStore();

    // Sequential IDs (1, 2, 3, ...)
    const nextId = orders.length > 0 ? orders[orders.length - 1].id + 1 : 1;

    const newOrder = {
      id: nextId,
      customer,
      items,
      quantity,
      priority: Number(priority) || 0,
      status: "queued",
      createdAt: new Date().toISOString(),
    };

    orders.push(newOrder);
    await writeStore(orders);

    console.log("✅ Order created:", newOrder);
    res.status(201).json(newOrder);
  } catch (err) {
    console.error("💥 Server error:", err);
    res.status(500).json({ error: "Internal server error" });
  }
});

// ✅ Delete order
app.delete("/api/order/:id", async (req, res) => {
  const id = parseInt(req.params.id);
  let orders = await readStore();
  orders = orders.filter(o => o.id !== id);
  await writeStore(orders);
  res.json({ message: `Order ${id} deleted` });
});

// ✅ Process next order (change status only)
app.post("/api/processNext", async (req, res) => {
  const orders = await readStore();
  const next = orders
    .filter(o => o.status === "queued")
    .sort((a, b) => b.priority - a.priority)[0];

  if (!next) {
    return res.json({ message: "No queued orders left." });
  }

  next.status = "processed";
  await writeStore(orders);
  res.json(next);
});

// ---------------- SERVER START ----------------
app.listen(PORT, () => {
  console.log(`✅ API running on http://localhost:${PORT}`);
});
