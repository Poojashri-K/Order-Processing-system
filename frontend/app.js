const API_BASE = "http://localhost:3000/api";

async function loadOrders() {
  const res = await fetch(`${API_BASE}/order`);
  const orders = await res.json();

  const listDiv = document.getElementById("ordersList");
  listDiv.innerHTML = "";

  if (orders.length === 0) {
    listDiv.innerHTML = "<p>No orders yet.</p>";
    return;
  }

  orders.forEach((order) => {
    const div = document.createElement("div");
    div.className = "order-item";
    div.innerHTML = `
      <p><strong>Order #${order.id}</strong> — ${order.customer}</p>
      <p>Items: ${Array.isArray(order.items) ? order.items.join(", ") : order.items}</p>
      <p>Quantity: ${order.quantity}</p>
      <p>Priority: ${order.priority}</p>
      <p>Status: ${order.status}</p>
      <button class="delete-btn" data-id="${order.id}">Delete</button>
    `;
    listDiv.appendChild(div);
  });

  // 🗑️ Delete functionality
  document.querySelectorAll(".delete-btn").forEach((btn) => {
    btn.addEventListener("click", async (e) => {
      const id = e.target.getAttribute("data-id");
      if (confirm(`Delete order #${id}?`)) {
        await fetch(`${API_BASE}/order/${id}`, { method: "DELETE" });
        await loadOrders();
      }
    });
  });
}

document.addEventListener("DOMContentLoaded", () => {
  document.getElementById("createBtn").onclick = async () => {
    const customer = document.getElementById("customer").value.trim();
    const items = document
      .getElementById("items")
      .value.split(",")
      .map((i) => i.trim())
      .filter(Boolean);
    const quantity = document.getElementById("quantity").value.trim();
    const priority = document.getElementById("priority").value.trim();

    if (!customer || items.length === 0 || !quantity) {
      alert("⚠️ Please fill all fields!");
      return;
    }

    const res = await fetch(`${API_BASE}/order`, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ customer, items, quantity, priority }),
    });

    if (!res.ok) {
      const err = await res.json().catch(() => ({}));
      alert(`❌ Failed to create order\n${err.error || ""}`);
      return;
    }

    const order = await res.json();
    alert(`✅ Order #${order.id} created for ${order.customer}`);

    // Clear inputs
    document.getElementById("customer").value = "";
    document.getElementById("items").value = "";
    document.getElementById("quantity").value = "";
    document.getElementById("priority").value = "";

    await loadOrders();
  };

  document.getElementById("refreshBtn").onclick = loadOrders;

  // 🚚 Process Next — mark as processed, don't remove
  document.getElementById("processBtn").onclick = async () => {
    const res = await fetch(`${API_BASE}/processNext`, { method: "POST" });
    const result = await res.json();

    if (result.message) {
      alert(result.message);
      return;
    }

    alert(`🚚 Processing order #${result.id} (${result.customer})`);

    // Update status in UI instead of reload
    const orderDiv = [...document.querySelectorAll(".order-item")].find((div) =>
      div.querySelector("strong")?.textContent.includes(`#${result.id}`)
    );

    if (orderDiv) {
      const statusLine = [...orderDiv.querySelectorAll("p")].find((p) =>
        p.textContent.startsWith("Status:")
      );
      if (statusLine) statusLine.textContent = "Status: Processed ✅";
    }
  };

  loadOrders();
});
